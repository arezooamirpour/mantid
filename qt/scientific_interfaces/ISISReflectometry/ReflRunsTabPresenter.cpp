#include "ReflRunsTabPresenter.h"
#include "IReflMainWindowPresenter.h"
#include "IReflRunsTabView.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/CatalogManager.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidKernel/CatalogInfo.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/FacilityInfo.h"
#include "MantidKernel/UserCatalogInfo.h"
#include "MantidQtWidgets/Common/AlgorithmRunner.h"
#include "MantidQtWidgets/Common/DataProcessorUI/Command.h"
#include "MantidQtWidgets/Common/DataProcessorUI/DataProcessorPresenter.h"
#include "MantidQtWidgets/Common/ParseKeyValueString.h"
#include "MantidQtWidgets/Common/ProgressPresenter.h"
#include "ReflCatalogSearcher.h"
#include "ReflFromStdStringMap.h"
#include "ReflLegacyTransferStrategy.h"
#include "ReflMeasureTransferStrategy.h"
#include "ReflNexusMeasurementItemSource.h"
#include "ReflSearchModel.h"

#include <QStringList>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

#include "Reduction/Slicing.h"

using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace MantidQt::MantidWidgets;
using namespace MantidQt::MantidWidgets::DataProcessor;

namespace MantidQt {
namespace CustomInterfaces {

namespace {
Mantid::Kernel::Logger g_log("Reflectometry GUI");

QStringList fromStdStringVector(std::vector<std::string> const &inVec) {
  QStringList outVec;
  std::transform(inVec.begin(), inVec.end(), std::back_inserter(outVec),
                 &QString::fromStdString);
  return outVec;
}
}

/** Constructor
* @param mainView :: [input] The view we're managing
* @param progressableView :: [input] The view reporting progress
* @param tablePresenters :: [input] The data processor presenters
* @param searcher :: [input] The search implementation
*/
ReflRunsTabPresenter::ReflRunsTabPresenter(
    IReflRunsTabView *mainView, ProgressableView *progressableView,
    BatchPresenterFactory makeBatchPresenter,
    std::vector<std::string> const &instruments, int defaultInstrumentIndex,
    boost::shared_ptr<IReflSearcher> searcher)
    : m_view(mainView), m_progressView(progressableView),
      m_makeBatchPresenter(makeBatchPresenter), m_mainPresenter(nullptr),
      m_searcher(searcher), m_instrumentChanged(false) {

  assert(m_view != nullptr);
  m_view->subscribe(this);
  for (const auto &tableView : m_view->tableViews())
    m_tablePresenters.emplace_back(m_makeBatchPresenter(tableView));

  m_view->setInstrumentList(instruments, defaultInstrumentIndex);

  // If we don't have a searcher yet, use ReflCatalogSearcher
  if (!m_searcher)
    m_searcher.reset(new ReflCatalogSearcher());
}

ReflRunsTabPresenter::~ReflRunsTabPresenter() {}

/** Accept a main presenter
* @param mainPresenter :: [input] A main presenter
*/
void ReflRunsTabPresenter::acceptMainPresenter(
    IReflMainWindowPresenter *mainPresenter) {
  m_mainPresenter = mainPresenter;
  // Register this presenter as the workspace receiver
  // When doing so, the inner presenters will notify this
  // presenter with the list of commands

  // for (const auto &presenter : m_tablePresenters)
  //  presenter->accept(this);

  // Note this must be done here since notifying the gdpp of its view
  // will cause it to request settings only accessible via the main
  // presenter.
}

void ReflRunsTabPresenter::settingsChanged(int group) {
  assert(static_cast<std::size_t>(group) < m_tablePresenters.size());
  // m_tablePresenters[group]->settingsChanged();
}

/**
Used by the view to tell the presenter something has changed
*/
void ReflRunsTabPresenter::notify(IReflRunsTabPresenter::Flag flag) {
  switch (flag) {
  case IReflRunsTabPresenter::SearchFlag:
    search();
    break;
  case IReflRunsTabPresenter::NewAutoreductionFlag:
    autoreduce(true);
    break;
  case IReflRunsTabPresenter::ResumeAutoreductionFlag:
    autoreduce(false);
    break;
  case IReflRunsTabPresenter::ICATSearchCompleteFlag: {
    auto algRunner = m_view->getAlgorithmRunner();
    IAlgorithm_sptr searchAlg = algRunner->getAlgorithm();
    populateSearch(searchAlg);
    break;
  }
  case IReflRunsTabPresenter::TransferFlag:
    transfer();
    break;
  case IReflRunsTabPresenter::InstrumentChangedFlag:
    changeInstrument();
    break;
  case IReflRunsTabPresenter::GroupChangedFlag:
    pushCommands();
    break;
  }
  // Not having a 'default' case is deliberate. gcc issues a warning if there's
  // a flag we aren't handling.
}

void ReflRunsTabPresenter::completedGroupReductionSuccessfully(
    GroupData const &group, std::string const &workspaceName) {
  m_mainPresenter->completedGroupReductionSuccessfully(group, workspaceName);
}

void ReflRunsTabPresenter::completedRowReductionSuccessfully(
    GroupData const &group, std::string const &workspaceNames) {
  m_mainPresenter->completedRowReductionSuccessfully(group, workspaceNames);
}

/** Pushes the list of commands (actions) */
void ReflRunsTabPresenter::pushCommands() {

  m_view->clearCommands();

  // The expected number of commands
  const size_t nCommands = 31;
  auto commands = std::vector<MantidWidgets::DataProcessor::Command_uptr>();
  //      m_tablePresenters.at(m_view->getSelectedGroup())->publishCommands();
  if (commands.size() != nCommands) {
    throw std::runtime_error("Invalid list of commands");
  }
  // The index at which "row" commands start
  const size_t rowCommStart = 10u;
  // We want to have two menus
  // Populate the "Reflectometry" menu
  std::vector<MantidWidgets::DataProcessor::Command_uptr> tableCommands;
  for (size_t i = 0; i < rowCommStart; i++)
    tableCommands.push_back(std::move(commands[i]));
  m_view->setTableCommands(std::move(tableCommands));
  // Populate the "Edit" menu
  std::vector<MantidWidgets::DataProcessor::Command_uptr> rowCommands;
  for (size_t i = rowCommStart; i < nCommands; i++)
    rowCommands.push_back(std::move(commands[i]));
  m_view->setRowCommands(std::move(rowCommands));
}

/** Searches for runs that can be used */
void ReflRunsTabPresenter::search() {
  auto const searchString = m_view->getSearchString();
  // Don't bother searching if they're not searching for anything
  if (searchString.empty())
    return;

  // This is breaking the abstraction provided by IReflSearcher, but provides a
  // nice usability win
  // If we're not logged into a catalog, prompt the user to do so
  if (CatalogManager::Instance().getActiveSessions().empty()) {
    try {
      std::stringstream pythonSrc;
      pythonSrc << "try:\n";
      pythonSrc << "  algm = CatalogLoginDialog()\n";
      pythonSrc << "except:\n";
      pythonSrc << "  pass\n";
      m_mainPresenter->runPythonAlgorithm(pythonSrc.str());
    } catch (std::runtime_error &e) {
      m_mainPresenter->giveUserCritical(
          "Error Logging in:\n" + std::string(e.what()), "login failed");
    }
  }
  std::string sessionId;
  // check to see if we have any active sessions for ICAT
  if (!CatalogManager::Instance().getActiveSessions().empty()) {
    // we have an active session, so grab the ID
    sessionId =
        CatalogManager::Instance().getActiveSessions().front()->getSessionId();
  } else {
    // there are no active sessions, we return here to avoid an exception
    m_mainPresenter->giveUserInfo(
        "Error Logging in: Please press 'Search' to try again.",
        "Login Failed");
    return;
  }
  auto algSearch = AlgorithmManager::Instance().create("CatalogGetDataFiles");
  algSearch->initialize();
  algSearch->setChild(true);
  algSearch->setLogging(false);
  algSearch->setProperty("OutputWorkspace", "_ReflSearchResults");
  algSearch->setProperty("Session", sessionId);
  algSearch->setProperty("InvestigationId", searchString);
  auto algRunner = m_view->getAlgorithmRunner();
  algRunner->startAlgorithm(algSearch);
}

/** Populates the search results table
* @param searchAlg : [input] The search algorithm
*/
void ReflRunsTabPresenter::populateSearch(IAlgorithm_sptr searchAlg) {
  if (searchAlg->isExecuted()) {
    ITableWorkspace_sptr results = searchAlg->getProperty("OutputWorkspace");
    m_instrumentChanged = false;
    m_searchModel = ReflSearchModel_sptr(
        new ReflSearchModel(results, m_view->getSearchInstrument()));
    m_view->showSearch(m_searchModel);
  }
}

/** Searches ICAT for runs with given instrument and investigation id, transfers
* runs to table and processes them
* @param startNew : Boolean on whether to start a new autoreduction
*/
void ReflRunsTabPresenter::autoreduce(bool startNew) {
  m_autoSearchString = m_view->getSearchString();
  auto &tablePresenter = m_tablePresenters.at(m_view->getSelectedGroup());

  // If a new autoreduction is being made, we must remove all existing rows and
  // transfer the new ones (obtained by ICAT search) in
  //  if (startNew) {
  //    notify(IReflRunsTabPresenter::ICATSearchCompleteFlag);
  //
  //    // Select all rows / groups in existing table and delete them
  //    tablePresenter->notify(DataProcessorPresenter::SelectAllFlag);
  //    tablePresenter->notify(DataProcessorPresenter::DeleteGroupFlag);
  //
  //    // Select and transfer all rows to the table
  //    m_view->setAllSearchRowsSelected();
  //    if (m_view->getSelectedSearchRows().size() > 0)
  //      transfer();
  //  }
  //
  //  tablePresenter->notify(DataProcessorPresenter::SelectAllFlag);
  //  if (tablePresenter->selectedParents().size() > 0)
  //    tablePresenter->notifyProcessRequested();
}

struct SearchResult {
  std::string run, description, location;
};

std::vector<SearchResult>
resultsFromSearchModel(boost::shared_ptr<ReflSearchModel> searchResultModel,
                       std::set<int> selectedRows) {
  auto results = std::vector<SearchResult>();
  results.reserve(selectedRows.size());

  for (const auto &row : selectedRows) {
    auto run = searchResultModel->data(searchResultModel->index(row, 0))
                   .toString()
                   .toStdString();

    auto description = searchResultModel->data(searchResultModel->index(row, 1))
                           .toString()
                           .toStdString();

    auto location = searchResultModel->data(searchResultModel->index(row, 2))
                        .toString()
                        .toStdString();

    results.emplace_back({run, description, location});
  }
  return results;
}

/** Transfers the selected runs in the search results to the processing table
* @return : The runs to transfer as a vector of maps
*/
void ReflRunsTabPresenter::transfer() {
  // Build the input for the transfer strategy
  auto selectedRows = m_view->getSelectedSearchRows();

  // Do not begin transfer if nothing is selected or if the transfer method does
  // not match the one used for populating search
  if (selectedRows.size() == 0) {
    m_mainPresenter->giveUserCritical(
        "Error: Please select at least one run to transfer.",
        "No runs selected");
    return;
  }

  ProgressPresenter progress(0, static_cast<double>(selectedRows.size()),
                             static_cast<int64_t>(selectedRows.size()),
                             this->m_progressView);

  auto results = resultsFromSearchModel(m_searchModel, selectedRows);

  auto jobs = Jobs(UnslicedReductionJobs());

  for (const auto &row : results) {
    static boost::regex descriptionFormatRegex("(.*)(th[:=]([0-9.]+))(.*)");
    boost::smatch matches;

    if (boost::regex_search(row.description, matches, descriptionFormatRegex)) {
      constexpr auto preThetaGroup = 1;
      constexpr auto thetaValueGroup = 3;
      constexpr auto postThetaGroup = 4;
      // We have theta. Let's get a clean description
      const auto theta = matches[thetaValueGroup].str();
      const auto preTheta = matches[preThetaGroup].str();
      const auto postTheta = matches[postThetaGroup].str();

      auto slicing = Slicing(boost::blank());
      auto resultRow =
          validateRowFromRunAndTheta(jobs, slicing, row.run, theta);
      if (resultRow.is_initialized()) {
        mergeRowIntoGroup(jobs, resultRow.get(), /*thetaTolerance=*/0.001,
                          preTheta, WorkspaceNameFactory(slicing));
      } else {
        // Add error to search model.
      }
    }
  }

  prettyPrintModel(jobs);

  /*
    TransferResults results = getTransferStrategy()->transferRuns(runs,
    progress);

    auto invalidRuns =
        results.getErrorRuns(); // grab our invalid runs from the transfer

    // iterate through invalidRuns to set the 'invalid transfers' in the search
    // model
    if (!invalidRuns.empty()) { // check if we have any invalid runs
      for (auto invalidRowIt = invalidRuns.begin();
           invalidRowIt != invalidRuns.end(); ++invalidRowIt) {
        auto &error = *invalidRowIt; // grab row from vector
        // iterate over row containing run number and reason why it's invalid
        for (auto errorRowIt = error.begin(); errorRowIt != error.end();
             ++errorRowIt) {
          const std::string runNumber = errorRowIt->first; // grab run number

          // iterate over rows that are selected in the search table
          for (auto rowIt = selectedRows.begin(); rowIt != selectedRows.end();
               ++rowIt) {
            const int row = *rowIt;
            // get the run number from that selected row
            const auto searchRun =
                m_searchModel->data(m_searchModel->index(row, 0))
                    .toString()
                    .toStdString();
            if (searchRun == runNumber) { // if search run number is the same as
                                          // our invalid run number

              // add this error to the member of m_searchModel that holds
    errors.
              m_searchModel->m_errors.push_back(error);
            }
          }
        }
      }
    }
  */
  // m_tablePresenters.at(m_view->getSelectedGroup())
  //    ->transfer(::MantidQt::CustomInterfaces::fromStdStringVectorMap(
  //        results.getTransferRuns()));
}

/**
* Select and make a transfer strategy on demand based. Pick up the
* user-provided transfer strategy to do this.
* @return new TransferStrategy
*/
std::unique_ptr<ReflTransferStrategy>
ReflRunsTabPresenter::getTransferStrategy() {
  std::unique_ptr<ReflTransferStrategy> rtnStrategy;
  if (m_currentTransferMethod == MeasureTransferMethod) {

    // We need catalog info overrides from the user-based config service
    std::unique_ptr<CatalogConfigService> catConfigService(
        makeCatalogConfigServiceAdapter(ConfigService::Instance()));

    // We make a user-based Catalog Info object for the transfer
    std::unique_ptr<ICatalogInfo> catInfo = make_unique<UserCatalogInfo>(
        ConfigService::Instance().getFacility().catalogInfo(),
        *catConfigService);

    // We are going to load from disk to pick up the meta data, so provide the
    // right repository to do this.
    std::unique_ptr<ReflMeasurementItemSource> source =
        make_unique<ReflNexusMeasurementItemSource>();

    // Finally make and return the Measure based transfer strategy.
    rtnStrategy = Mantid::Kernel::make_unique<ReflMeasureTransferStrategy>(
        std::move(catInfo), std::move(source));
    return rtnStrategy;
  } else if (m_currentTransferMethod == LegacyTransferMethod) {
    rtnStrategy = make_unique<ReflLegacyTransferStrategy>();
    return rtnStrategy;
  } else {
    throw std::runtime_error("Unknown tranfer method selected: " +
                             m_currentTransferMethod);
  }
}

/** Used to tell the presenter something has changed in the ADS
*
* @param workspaceList :: the list of table workspaces in the ADS that could be
* loaded into the interface
*/
void ReflRunsTabPresenter::notifyADSChanged(
    const QSet<QString> &workspaceList) {

  UNUSED_ARG(workspaceList);
  pushCommands();
  //  m_view->updateMenuEnabledState(
  //     m_tablePresenters.at(m_view->getSelectedGroup())->isProcessing());
}

/** Requests global pre-processing options. Options are supplied by
  * the main presenter and there can be multiple sets of options for different
  * columns that need to be preprocessed.
  * @return :: A map of the column name to the global pre-processing options
  * for that column
  * the main presenter.
  * @return :: Global pre-processing options
  */
ColumnOptionsQMap ReflRunsTabPresenter::getPreprocessingOptions() const {
  ColumnOptionsQMap result;
  assert(m_mainPresenter != nullptr &&
         "The main presenter must be set with acceptMainPresenter.");

  // Note that there are no options for the Run(s) column so just add
  // Transmission Run(s)
  auto transmissionOptions = OptionsQMap(
      m_mainPresenter->getTransmissionOptions(m_view->getSelectedGroup()));
  result["Transmission Run(s)"] = transmissionOptions;

  return result;
}

/** Requests global processing options. Options are supplied by the main
* presenter
* @return :: Global processing options
*/
OptionsQMap ReflRunsTabPresenter::getProcessingOptions() const {
  assert(m_mainPresenter != nullptr &&
         "The main presenter must be set with acceptMainPresenter.");
  return m_mainPresenter->getReductionOptions(m_view->getSelectedGroup());
}

/** Requests global post-processing options as a string. Options are supplied by
* the main
* presenter
* @return :: Global post-processing options as a string
*/
QString ReflRunsTabPresenter::getPostprocessingOptionsAsString() const {

  return QString::fromStdString(
      m_mainPresenter->getStitchOptions(m_view->getSelectedGroup()));
}

/** Requests time-slicing values. Values are supplied by the main presenter
* @return :: Time-slicing values
*/
QString ReflRunsTabPresenter::getTimeSlicingValues() const {
  return QString::fromStdString(
      m_mainPresenter->getTimeSlicingValues(m_view->getSelectedGroup()));
}

/** Requests time-slicing type. Type is supplied by the main presenter
* @return :: Time-slicing values
*/
QString ReflRunsTabPresenter::getTimeSlicingType() const {
  return QString::fromStdString(
      m_mainPresenter->getTimeSlicingType(m_view->getSelectedGroup()));
}

/** Requests transmission runs for a particular run angle. Values are supplied
* by the main presenter
* @return :: Transmission run(s) as a comma-separated list
*/
OptionsQMap ReflRunsTabPresenter::getOptionsForAngle(const double angle) const {
  return m_mainPresenter->getOptionsForAngle(m_view->getSelectedGroup(), angle);
}

/** Check whether there are per-angle transmission runs in the settings
 * @return :: true if there are per-angle transmission runs
 */
bool ReflRunsTabPresenter::hasPerAngleOptions() const {
  return m_mainPresenter->hasPerAngleOptions(m_view->getSelectedGroup());
}

/** Tells the view to update the enabled/disabled state of all relevant widgets
 * based on whether processing is in progress or not.
 * @param isProcessing :: true if processing is in progress
 *
 */
void ReflRunsTabPresenter::updateWidgetEnabledState(
    const bool isProcessing) const {
  // Update the menus
  m_view->updateMenuEnabledState(isProcessing);

  // Update specific buttons
  m_view->setAutoreduceButtonEnabled(!isProcessing);
  m_view->setTransferButtonEnabled(!isProcessing);
  m_view->setInstrumentComboEnabled(!isProcessing);
}

/** Tells view to update the enabled/disabled state of all relevant widgets
 * based on the fact that processing is not in progress
*/
void ReflRunsTabPresenter::pause() const { updateWidgetEnabledState(false); }

/** Tells view to update the enabled/disabled state of all relevant widgets
 * based on the fact that processing is in progress
*/
void ReflRunsTabPresenter::resume() const { updateWidgetEnabledState(true); }

/** Determines whether to start a new autoreduction. Starts a new one if the
* either the search number, transfer method or instrument has changed
* @return : Boolean on whether to start a new autoreduction
*/
bool ReflRunsTabPresenter::startNewAutoreduction() const {
  bool searchNumChanged = m_autoSearchString != m_view->getSearchString();
  return searchNumChanged || m_instrumentChanged;
}

/** Notifies main presenter that data reduction is confirmed to be paused
*/
void ReflRunsTabPresenter::confirmReductionPaused(int group) {
  m_mainPresenter->notifyReductionPaused(group);
}

/** Notifies main presenter that data reduction is confirmed to be resumed
*/
void ReflRunsTabPresenter::confirmReductionResumed(int group) {
  m_mainPresenter->notifyReductionResumed(group);
}

/** Changes the current instrument in the data processor widget. Also clears the
* and the table selection model and updates the config service, printing an
* information message
*/
void ReflRunsTabPresenter::changeInstrument() {
  auto const instrument = m_view->getSearchInstrument();
  m_mainPresenter->setInstrumentName(instrument);
  Mantid::Kernel::ConfigService::Instance().setString("default.instrument",
                                                      instrument);
  g_log.information() << "Instrument changed to " << instrument;
  m_instrumentChanged = true;
}
}
}
