#ifndef MANTIDQTCUSTOMINTERFACES_MUONANALYSIS_H_
#define MANTIDQTCUSTOMINTERFACES_MUONANALYSIS_H_

//----------------------
// Includes
//----------------------
#include "MantidQtCustomInterfaces/ui_MuonAnalysis.h"
#include "MantidQtAPI/UserSubWindow.h"

#include "MantidQtMantidWidgets/pythonCalc.h"
#include "MantidQtMantidWidgets/MWDiag.h"

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/MatrixWorkspace.h"
#include <map>

namespace MantidQt
{
namespace CustomInterfaces
{
class MuonAnalysis : public MantidQt::API::UserSubWindow
{
  Q_OBJECT

public:
  /// Name of the interface
  static std::string name() { return "MuonAnalysis"; }

public:
  /// Default Constructor
  MuonAnalysis(QWidget *parent = 0);

private slots:
  /// Exit the interface
  void exitClicked();

  /// Guess Alpha clicked
  void guessAlphaClicked();

  /// Input file changed
  void inputFileChanged();

  /// group table changed
  void groupTableChanged(int row, int column);

  // group table clicked
  void groupTableClicked(int row, int column);

  // pair table clicked
  void pairTableClicked(int row, int column);

  /// group table plot button
  void runGroupTablePlotButton();

  /// group table plot button
  void runPairTablePlotButton();

  /// change group table plotting choice
  void runGroupTablePlotChoice(const QString);

  // change pair table plotting choice
  void runPairTablePlotChoice(const QString str);

  /// Save grouping button
  void runSaveGroupButton();

  /// Load grouping button
  void runLoadGroupButton();

  /// Clear grouping button
  void runClearGroupingButton(); 

  /// User select instrument
  void userSelectInstrument(const QString& prefix);

  ///
  void runFrontPlotButton();

  ///
  void runFrontGroupGroupPairComboBox(int index);

  ///
  void runHomePeriodBox1(const QString& text);

  ///
  void muonAnalysisHelpClicked();

private:
  /// Initialize the layout
  virtual void initLayout();

  /// Set start up interface look
  void startUpLook();

  /// is grouping set
  bool isGroupingSet();

  /// Apply grouping specified in xml file to workspace
  void applyGroupingToWS( const std::string& wsName, std::string filename);

  /// Apply whatever grouping is specified in GUI tables to workspace
  void applyGroupingToWS( const std::string& wsName);

  /// Update front "group / group-pair" combo-box based on what the currentIndex now is
  void updateFront();

  /// Calculate number of detectors from string of type 1-3, 5, 10-15
  int numOfDetectors(std::string str) const;

  /// is string a number?
  bool MuonAnalysis::isNumber(const std::string& s) const;

  /// Clear tables and front combo box
  void clearTablesAndCombo();

  /// When no data loaded set various buttons etc to inactive
  void noDataAvailable();

  /// When data loaded set various buttons etc to active
  void nowDataAvailable();

  /// Is data loaded?
  bool m_dataLoaded;

  /// Return a none empty string if the data and group detector info are inconsistent
  QString dataAndTablesConsistent();

  /// Return number of pairs
  int numPairs();

  /// Return number of groups defined (not including pairs)
  int numGroups();

  /// Plot group
  void plotGroup(std::string& plotType);

  /// Plot pair
  void plotPair(std::string& plotType);

  //The form generated by Qt Designer
  Ui::MuonAnalysis m_uiForm;

  /// group plot functions
  QStringList m_groupPlotFunc;

  /// pair plot functions
  QStringList m_pairPlotFunc;

  /// The last directory that was viewed
  QString m_last_dir;

  /// name of workspace
  std::string m_workspace_name;

  /// period. when set to zero means no period (no workspace grouping)
  /// use it to create a workspace name
  int m_period;

  /// which group table row has the user last clicked on
  int m_groupTableRowInFocus;

  /// which pair table row has the user last clicked on
  int m_pairTableRowInFocus;

  /// used to test that a new filename has been entered 
  QString m_previousFilename;

  /// convert int to string
  std::string iToString(int i);

  /// group table plot choice
  std::string m_groupTablePlotChoice;

  /// pair table plot choice
  std::string m_pairTablePlotChoice;

  /// List of current group names 
  std::vector<std::string> m_groupNames;

  /// name for file to temperary store grouping
  std::string m_groupingTempFilename;

  /// Currently selected instrument
  QString m_curInterfaceSetup;

  ///
  ///std::map<int, QString> m_groupNameMap;

  //A reference to a logger
  static Mantid::Kernel::Logger & g_log;
};

}
}

#endif //MANTIDQTCUSTOMINTERFACES_MUONANALYSIS_H_
