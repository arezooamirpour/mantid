// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "CatalogSearcher.h"
#include "GUI/Runs/IRunsView.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/CatalogManager.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidQtWidgets/Common/AlgorithmRunner.h"

using namespace Mantid::API;

namespace MantidQt {
namespace CustomInterfaces {

namespace { // unnamed
void removeResultsWithoutFilenameExtension(ITableWorkspace_sptr results) {
  std::set<size_t> toRemove;
  for (size_t i = 0; i < results->rowCount(); ++i) {
    std::string &run = results->String(i, 0);

    // Too short to be more than ".raw or .nxs"
    if (run.size() < 5) {
      toRemove.insert(i);
    }
  }

  // Sets are sorted so if we go from back to front we won't trip over ourselves
  for (auto row = toRemove.rbegin(); row != toRemove.rend(); ++row)
    results->removeRow(*row);
}
} // unnamed namespace

CatalogSearcher::CatalogSearcher(IPythonRunner *pythonRunner, IRunsView *view)
    : m_pythonRunner(pythonRunner), m_view(view) {
  m_view->subscribeSearch(this);
}

void CatalogSearcher::subscribe(SearcherSubscriber *notifyee) {
  m_notifyee = notifyee;
}

ITableWorkspace_sptr CatalogSearcher::search(const std::string &text) {
  auto algSearch = createSearchAlgorithm(text);
  algSearch->execute();
  ITableWorkspace_sptr results = algSearch->getProperty("OutputWorkspace");
  // Now, tidy up the data
  removeResultsWithoutFilenameExtension(results);
  return results;
}

bool CatalogSearcher::startSearchAsync(const std::string &text) {
  if (!logInToCatalog())
    return false;

  auto algSearch = createSearchAlgorithm(text);
  auto algRunner = m_view->getAlgorithmRunner();
  algRunner->startAlgorithm(algSearch);

  return true;
}

void CatalogSearcher::notifySearchComplete() {
  auto algRunner = m_view->getAlgorithmRunner();
  IAlgorithm_sptr searchAlg = algRunner->getAlgorithm();

  // TODO: still notify if results are empty in case the notifyee needs to take
  // action e.g. restarting polling for autoreduction.
  if (!searchAlg->isExecuted())
    return;

  // TODO: pass the results as a search model instead of table workspace?
  ITableWorkspace_sptr results = searchAlg->getProperty("OutputWorkspace");
  m_notifyee->notifySearchResults(results);
}

bool CatalogSearcher::hasActiveSession() const {
  auto sessions = CatalogManager::Instance().getActiveSessions();
  return !sessions.empty();
}

/** Log in to the catalog
 * @returns : true if login succeeded
 */
bool CatalogSearcher::logInToCatalog() {
  if (hasActiveSession())
    return true;

  try {
    std::stringstream pythonSrc;
    pythonSrc << "try:\n";
    pythonSrc << "  algm = CatalogLoginDialog()\n";
    pythonSrc << "except:\n";
    pythonSrc << "  pass\n";
    m_pythonRunner->runPythonAlgorithm(pythonSrc.str());
  } catch (std::runtime_error &e) {
    return false;
  }

  // Check we logged in ok
  return hasActiveSession();
}

std::string CatalogSearcher::activeSessionId() const {
  auto sessions = CatalogManager::Instance().getActiveSessions();
  if (sessions.empty())
    throw std::runtime_error("You are not logged into any catalogs.");

  return sessions.front()->getSessionId();
}

IAlgorithm_sptr
CatalogSearcher::createSearchAlgorithm(const std::string &text) {
  if (!logInToCatalog())
    throw std::runtime_error("Catalog login failed: ");

  auto sessionId = activeSessionId();

  auto algSearch = AlgorithmManager::Instance().create("CatalogGetDataFiles");
  algSearch->initialize();
  algSearch->setChild(true);
  algSearch->setLogging(false);
  algSearch->setProperty("Session", sessionId);
  algSearch->setProperty("InvestigationId", text);
  algSearch->setProperty("OutputWorkspace", "_ReflSearchResults");

  return algSearch;
}
} // namespace CustomInterfaces
} // namespace MantidQt