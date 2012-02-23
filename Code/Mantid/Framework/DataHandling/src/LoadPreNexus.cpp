/*WIKI*
Workflow algorithm to load all of the preNeXus files.
*WIKI*/

#include <exception>
#include <fstream>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/SAX/InputSource.h>
#include "MantidAPI/IEventWorkspace.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/LoadAlgorithmFactory.h"
#include "MantidDataHandling/LoadPreNexus.h"
#include "MantidKernel/System.h"
#include "MantidKernel/VisibleWhenProperty.h"
#include "MantidAPI/TableRow.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using std::size_t;
using std::string;
using std::vector;

namespace Mantid
{
namespace DataHandling
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(LoadPreNexus)
  DECLARE_LOADALGORITHM(LoadPreNexus)

  static const string RUNINFO_PARAM("Filename");
  static const string MAP_PARAM("MappingFilename");


  //----------------------------------------------------------------------------------------------
  /// Constructor
  LoadPreNexus::LoadPreNexus()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /// Destructor
  LoadPreNexus::~LoadPreNexus()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// @copydoc Mantid::API::IAlgorithm::name()
  const std::string LoadPreNexus::name() const
  {
    return "LoadPreNexus";
  }
  
  /// @copydoc Mantid::API::IAlgorithm::version()
  int LoadPreNexus::version() const
  {
    return 1;
  }
  
  /// @copydoc Mantid::API::IAlgorithm::category()
  const std::string LoadPreNexus::category() const
  {
    return "DataHandling\\PreNexus;Workflow\\DataHandling";
  }

  //----------------------------------------------------------------------------------------------
  /// @copydoc Mantid::API::Algorithm::initDocs()
  void LoadPreNexus::initDocs()
  {
    this->setWikiSummary("Load a collection of PreNexus files.");
    this->setOptionalMessage("Load a collection of PreNexus files.");
  }

  //----------------------------------------------------------------------------------------------
  /// @copydoc Mantid::API::IDataFileChecker::filePropertyName()
  const char * LoadPreNexus::filePropertyName() const
  {
    return RUNINFO_PARAM.c_str();
  }

  /// @copydoc Mantid::API::IDataFileChecker::quickFileCheck
  bool LoadPreNexus::quickFileCheck(const std::string& filePath,size_t nread,const file_header& header)
  {
    UNUSED_ARG(nread);
    UNUSED_ARG(header);

    std::string ext = extension(filePath);
    return (ext.rfind("_runinfo.xml") != std::string::npos);
  }

  /// @copydoc Mantid::API::IDataFileChecker::fileCheck
  int LoadPreNexus::fileCheck(const std::string& filePath)
  {
    std::string ext = extension(filePath);

    if (ext.rfind("_runinfo.xml") != std::string::npos)
      return 80;
    else
      return 0;
  }

  //----------------------------------------------------------------------------------------------
  /// @copydoc Mantid::API::Algorithm::init()
  void LoadPreNexus::init()
  {
    // runfile to read in
    declareProperty(new FileProperty(RUNINFO_PARAM, "", FileProperty::Load, "_runinfo.xml"),
        "The name of the runinfo file to read, including its full or relative path.");

    // copied (by hand) from LoadEventPreNexus2
    declareProperty(new FileProperty(MAP_PARAM, "", FileProperty::OptionalLoad, ".dat"),
        "File containing the pixel mapping (DAS pixels to pixel IDs) file (typically INSTRUMENT_TS_YYYY_MM_DD.dat). The filename will be found automatically if not specified.");
    BoundedValidator<int> *mustBePositive = new BoundedValidator<int>();
    mustBePositive->setLower(1);
    declareProperty("MaxChunkSize", EMPTY_DBL(),
        "Get chunking strategy for chunks with this number of Gbytes.");
    declareProperty("ChunkNumber", EMPTY_INT(), mustBePositive,
        "If loading the file by sections ('chunks'), this is the section number of this execution of the algorithm.");
    declareProperty("TotalChunks", EMPTY_INT(), mustBePositive->clone(),
        "If loading the file by sections ('chunks'), this is the total number of sections.");
    // TotalChunks is only meaningful if ChunkNumber is set
    // Would be nice to be able to restrict ChunkNumber to be <= TotalChunks at validation
    setPropertySettings("TotalChunks", new VisibleWhenProperty(this, "ChunkNumber", IS_NOT_DEFAULT));
    std::vector<std::string> propOptions;
    propOptions.push_back("Auto");
    propOptions.push_back("Serial");
    propOptions.push_back("Parallel");
    declareProperty("UseParallelProcessing", "Auto",new ListValidator(propOptions),
        "Use multiple cores for loading the data?\n"
        "  Auto: Use serial loading for small data sets, parallel for large data sets.\n"
        "  Serial: Use a single core.\n"
        "  Parallel: Use all available cores.");

    declareProperty(new PropertyWithValue<bool>("LoadMonitors", true, Direction::Input),
                    "Load the monitors from the file.");


    declareProperty(new WorkspaceProperty<API::Workspace>("OutputWorkspace","",Direction::Output), "An output workspace.");
  }

  //----------------------------------------------------------------------------------------------
  /// @copydoc Mantid::API::Algorithm::exec()
  void LoadPreNexus::exec()
  {
    string runinfo = this->getPropertyValue(RUNINFO_PARAM);
    string mapfile = this->getPropertyValue(MAP_PARAM);
    int chunkTotal = this->getProperty("TotalChunks");
    int chunkNumber = this->getProperty("ChunkNumber");
    double maxChunk = this->getProperty("MaxChunkSize");
    if (!isEmpty(maxChunk))
    {
      int NChunks = determineChunking(runinfo,maxChunk);
      NChunks++;  // For python range
      Mantid::API::ITableWorkspace_sptr strategy = Mantid::API::WorkspaceFactory::Instance().createTable("TableWorkspace");
      strategy->addColumn("int","ChunkNumber");
      strategy->addColumn("int","TotalChunks");

      for (int i = 1; i <= NChunks; i++) 
      {
        Mantid::API::TableRow row = strategy->appendRow();
        row << i << NChunks;
      }
      
      this->setProperty("OutputWorkspace", strategy);
      return;
    }
    if ( isEmpty(chunkTotal) || isEmpty(chunkNumber))
    {
      chunkNumber = EMPTY_INT();
      chunkTotal = EMPTY_INT();
    }
    else
    {
      if (chunkNumber > chunkTotal)
        throw std::out_of_range("ChunkNumber cannot be larger than TotalChunks");
    }
    string useParallel = this->getProperty("UseParallelProcessing");
    string wsname = this->getProperty("OutputWorkspace");
    bool loadmonitors = this->getProperty("LoadMonitors");

    // determine the event file names
    Progress prog(this, 0., .1, 1);
    vector<string> eventFilenames;
    string dataDir;
    this->parseRuninfo(runinfo, dataDir, eventFilenames);
    prog.doReport("parsed runinfo file");

    // do math for the progress bar
    size_t numFiles = eventFilenames.size() + 1; // extra 1 is nexus logs
    if (loadmonitors)
      numFiles++;
    double prog_start = .1;
    double prog_delta = (1.-prog_start)/static_cast<double>(numFiles);

    // load event files
    IEventWorkspace_sptr outws;
    string temp_wsname;

    for (size_t i = 0; i < eventFilenames.size(); i++) {
      if (i == 0)
        temp_wsname = wsname;
      else
        temp_wsname = "__" + wsname + "_temp__";

      IAlgorithm_sptr alg = this->createSubAlgorithm("LoadEventPreNexus", prog_start, prog_start+prog_delta);
      alg->setProperty("EventFilename", dataDir + eventFilenames[i]);
      alg->setProperty("MappingFilename", mapfile);
      alg->setProperty("ChunkNumber", chunkNumber);
      alg->setProperty("TotalChunks", chunkTotal);
      alg->setProperty("UseParallelProcessing", useParallel);
      alg->setPropertyValue("OutputWorkspace", temp_wsname);
      alg->executeAsSubAlg();
      prog_start += prog_delta;

      if (i == 0)
      {
        outws = alg->getProperty("OutputWorkspace");
      }
      else
      {
        IEventWorkspace_sptr tempws = alg->getProperty("OutputWorkspace");
        // clean up properties before adding data
        Run & run = tempws->mutableRun();
        if (run.hasProperty("gd_prtn_chrg"))
          run.removeProperty("gd_prtn_chrg");
        if (run.hasProperty("proton_charge"))
          run.removeProperty("proton_charge");

        outws += tempws;
      }
    }


    // load the logs
    this->runLoadNexusLogs(runinfo, dataDir, outws, prog_start, prog_start+prog_delta);
    prog_start += prog_delta;

    // publish output workspace
    this->setProperty("OutputWorkspace", outws);

    // load the monitor
    if (loadmonitors)
    {
      this->runLoadMonitors(prog_start, 1.);
    }
  }

  /**
   * Parse the runinfo file to find the names of the neutron event files.
   *
   * @param runinfo Runinfo file with full path.
   * @param dataDir Directory where the runinfo file lives.
   * @param eventFilenames vector of all possible event files. This is filled by the algorithm.
   */
  void LoadPreNexus::parseRuninfo(const string &runinfo, string &dataDir, vector<string> &eventFilenames)
  {
    eventFilenames.clear();

    // Create a Poco Path object for runinfo filename
    Poco::Path runinfoPath(runinfo, Poco::Path::PATH_GUESS);
    // Now lets get the directory
    Poco::Path dirPath(runinfoPath.parent());
    dataDir = dirPath.absolute().toString();
    g_log.debug() << "Data directory \"" << dataDir << "\"\n";

    std::ifstream in(runinfo.c_str());
    Poco::XML::InputSource src(in);

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> pDoc = parser.parse(&src);

    Poco::XML::NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ELEMENT);
    Poco::XML::Node* pNode = it.nextNode(); // root node
    while (pNode)
    {
      if (pNode->nodeName() == "RunInfo") // standard name for this type
      {
        pNode = pNode->firstChild();
        while (pNode)
        {
          if (pNode->nodeName() == "FileList")
          {
            pNode = pNode->firstChild();
            while (pNode)
            {
              if (pNode->nodeName() == "DataList")
              {
                pNode = pNode->firstChild();
                while (pNode)
                {
                  if (pNode->nodeName() == "scattering")
                  {
                    Poco::XML::Element* element = static_cast<Poco::XML::Element*> (pNode);
                    eventFilenames.push_back(element->getAttribute("name"));
                  }
                  pNode = pNode->nextSibling();
                }
              }
              else // search for DataList
                pNode = pNode->nextSibling();
            }
          }
          else // search for FileList
            pNode = pNode->nextSibling();
        }
      }
      else // search for RunInfo
        pNode = pNode->nextSibling();
    }

    // report the results to the log
    if (eventFilenames.size() == 1)
    {
      g_log.debug() << "Found 1 event file: \"" << eventFilenames[0] << "\"\n";
    }
    else
    {
      g_log.debug() << "Found " << eventFilenames.size() << " event files:";
      for (size_t i = 0; i < eventFilenames.size(); i++)
      {
        g_log.debug() << "\"" << eventFilenames[i] << "\" ";
      }
      g_log.debug() << "\n";
    }
  }

  /**
   * Load logs from a nexus file onto the workspace.
   *
   * @param runinfo Runinfo file with full path.
   * @param dataDir Directory where the runinfo file lives.
   * @param wksp Workspace to add the logs to.
   * @param prog_start Starting position for the progress bar.
   * @param prog_stop Ending position for the progress bar.
   */
  void LoadPreNexus::runLoadNexusLogs(const string &runinfo, const string &dataDir,
                                      IEventWorkspace_sptr wksp, const double prog_start, const double prog_stop)
  {
    // determine the name of the file "inst_run"
    string shortName = runinfo.substr(dataDir.size());
    shortName = shortName.substr(0, shortName.find("_runinfo.xml"));
    g_log.debug() << "SHORTNAME = \"" << shortName << "\"\n";

    // put together a list of possible locations
    vector<string> possibilities;
    possibilities.push_back(dataDir + shortName + "_event.nxs"); // next to runinfo
    possibilities.push_back(dataDir + shortName + "_histo.nxs");
    possibilities.push_back(dataDir + shortName + ".nxs");
    possibilities.push_back(dataDir + "../NeXus/" + shortName + "_event.nxs"); // in NeXus directory
    possibilities.push_back(dataDir + "../NeXus/" + shortName + "_histo.nxs");
    possibilities.push_back(dataDir + "../NeXus/" + shortName + ".nxs");

    // run the algorithm
    bool loadedLogs = false;
    for (size_t i = 0; i < possibilities.size(); i++)
    {
      if (Poco::File(possibilities[i]).exists())
      {
        g_log.information() << "Loading logs from \"" << possibilities[i] << "\"\n";
        IAlgorithm_sptr alg = this->createSubAlgorithm("LoadNexusLogs", prog_start, prog_stop);
        alg->setProperty("Workspace", wksp);
        alg->setProperty("Filename", possibilities[i]);
        alg->setProperty("OverwriteLogs", false);
        alg->executeAsSubAlg();
        loadedLogs = true;
        break;
      }
    }

    if (!loadedLogs)
      g_log.notice() << "Did not find a nexus file to load logs from\n";
  }

  /**
   * Load the monitor files.
   *
   * @param prog_start Starting position for the progress bar.
   * @param prog_stop Ending position for the progress bar.
   */
  void LoadPreNexus::runLoadMonitors(const double prog_start, const double prog_stop)
  {
    std::string mon_wsname = this->getProperty("OutputWorkspace");
    mon_wsname.append("_monitors");

    try{
    IAlgorithm_sptr alg = this->createSubAlgorithm("LoadPreNexusMonitors", prog_start, prog_stop);
    alg->setPropertyValue("RunInfoFilename", this->getProperty(RUNINFO_PARAM));
    alg->setPropertyValue("OutputWorkspace", mon_wsname);
    alg->executeAsSubAlg();
    MatrixWorkspace_sptr mons = alg->getProperty("OutputWorkspace");
    this->declareProperty(new WorkspaceProperty<>("MonitorWorkspace",
        mon_wsname, Direction::Output), "Monitors from the Event NeXus file");
    this->setProperty("MonitorWorkspace", mons);
    }
    catch (std::exception &e)
    {
      g_log.warning() << "Failed to load monitors: " << e.what() << "\n";
    }
  }

  int LoadPreNexus::determineChunking(const std::string& filename, double maxChunkSize)
  {
    vector<string> eventFilenames;
    string dataDir;
    this->parseRuninfo(filename, dataDir, eventFilenames);
    double filesize = 0;
    for (size_t i = 0; i < eventFilenames.size(); i++) {
      filesize += static_cast<double>(Poco::File(dataDir + eventFilenames[i]).getSize())/(1024.0*1024.0*1024.0);
    }
    if (filesize == 0)
      return 0;
    int numChunks = static_cast<int>(filesize/maxChunkSize);
    if (numChunks > 0)
      return numChunks; // high end is exclusive
    else
      return 1;

  }


} // namespace Mantid
} // namespace DataHandling
