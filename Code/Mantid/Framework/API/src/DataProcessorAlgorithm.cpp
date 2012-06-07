#include "MantidAPI/DataProcessorAlgorithm.h"
#include "MantidAPI/AlgorithmProperty.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/IEventWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidAPI/FileFinder.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/PropertyManagerDataService.h"
#include "MantidKernel/PropertyManager.h"
#include <stdexcept>
#ifdef MPI_BUILD
#include <boost/mpi.hpp>
#endif

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace API
{


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  DataProcessorAlgorithm::DataProcessorAlgorithm()
  {
    m_loadAlg = "Load";
    m_accumulateAlg = "Plus";
    m_useMPI = false;
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  DataProcessorAlgorithm::~DataProcessorAlgorithm()
  {
  }

  //----------------------------------------------------------------------------------------------
  void DataProcessorAlgorithm::setLoadAlg(const std::string &alg)
  {
    if (alg.empty())
      throw std::invalid_argument("Cannot set load algorithm to empty string");
    m_loadAlg = alg;
  }

  void DataProcessorAlgorithm::setAccumAlg(const std::string &alg)
  {
    if (alg.empty())
      throw std::invalid_argument("Cannot set accumulate algorithm to empty string");
    m_accumulateAlg = alg;
  }

  ITableWorkspace_sptr DataProcessorAlgorithm::determineChunk()
  {
    throw std::runtime_error("DataProcessorAlgorithm::determineChunk is not implemented");
  }

  void DataProcessorAlgorithm::loadChunk()
  {

    throw std::runtime_error("DataProcessorAlgorithm::loadChunk is not implemented");
  }

  Workspace_sptr DataProcessorAlgorithm::assemble(const std::string &partialWSName, const std::string &outputWSName)
  {
#ifdef MPI_BUILD
    Workspace_sptr partialWS = AnalysisDataService::Instance().retrieve(partialWSName);
    IAlgorithm_sptr gatherAlg = createSubAlgorithm("GatherWorkspaces");
    gatherAlg->setAlwaysStoreInADS(true);
    gatherAlg->setProperty("InputWorkspace", partialWS);
    gatherAlg->setPropertyValue("OutputWorkspace", outputWSName);
    gatherAlg->execute();
#endif
    Workspace_sptr outputWS = AnalysisDataService::Instance().retrieve(outputWSName);
    return outputWS;
  }

  void DataProcessorAlgorithm::saveNexus(const std::string &outputWSName,
      const std::string &outputFile)
  {

    bool saveOutput = true;
#ifdef MPI_BUILD
    if(boost::mpi::communicator().rank()>0) saveOutput = false;
#endif

    if (saveOutput && outputFile.size() > 0)
    {
      IAlgorithm_sptr saveAlg = createSubAlgorithm("SaveNexus");
      saveAlg->setPropertyValue("Filename", outputFile);
      saveAlg->setPropertyValue("InputWorkspace", outputWSName);
      saveAlg->execute();
    }
  }

  /// Determine what kind of input data we have and load it
  //TODO: Chunking, MPI, etc...
  Workspace_sptr DataProcessorAlgorithm::load(const std::string &inputData)
  {
    Workspace_sptr inputWS;
    std::string outputWSName = inputData;

    // First, check whether we have the name of an existing workspace
    if (AnalysisDataService::Instance().doesExist(inputData))
    {
      Workspace_sptr inputWS = AnalysisDataService::Instance().retrieve(inputData);
    }
    else
    {
      std::string foundFile = FileFinder::Instance().getFullPath(inputData);
      if (foundFile.empty())
      {
        // Get facility extensions
        FacilityInfo facilityInfo = ConfigService::Instance().getFacility();
        const std::vector<std::string>  facilityExts = facilityInfo.extensions();
        foundFile = FileFinder::Instance().findRun(inputData, facilityExts);
      }

      if (!foundFile.empty())
      {
        IAlgorithm_sptr loadAlg = createSubAlgorithm(m_loadAlg);
        loadAlg->setProperty("Filename", foundFile);
        loadAlg->setAlwaysStoreInADS(true);

        // Set up MPI if available
#ifdef MPI_BUILD
        // First, check whether the loader allows use to chunk the data
        if (loadAlg->existsProperty("ChunkNumber")
            && loadAlg->existsProperty("TotalChunks"))
        {
          m_useMPI = true;
          // The communicator containing all processes
          boost::mpi::communicator world;
          g_log.notice() << "Chunk/Total: " << world.rank()+1 << "/" << world.size() << std::endl;
          loadAlg->setPropertyValue("OutputWorkspace", outputWSName);
          loadAlg->setProperty("ChunkNumber", world.rank()+1);
          loadAlg->setProperty("TotalChunks", world.size());
        }
#else
        loadAlg->setPropertyValue("OutputWorkspace", outputWSName);
#endif
        loadAlg->execute();

        Workspace_sptr inputMatrixWS = AnalysisDataService::Instance().retrieve(outputWSName);
      }
    }
    return inputWS;
  }

  /// Get the property manager object of a given name from the property manager
  /// data service, or create a new one.
  boost::shared_ptr<PropertyManager>
  DataProcessorAlgorithm::getProcessProperties(const std::string &propertyManager)
  {
    boost::shared_ptr<PropertyManager> processProperties;
    if (PropertyManagerDataService::Instance().doesExist(propertyManager))
    {
      processProperties = PropertyManagerDataService::Instance().retrieve(propertyManager);
    }
    else
    {
      g_log.notice() << "Could not find property manager" << std::endl;
      processProperties = boost::make_shared<PropertyManager>();
      PropertyManagerDataService::Instance().addOrReplace(propertyManager, processProperties);
    }
    return processProperties;
  }

  std::vector<std::string> DataProcessorAlgorithm::splitInput(const std::string & input)
  {
    UNUSED_ARG(input);
    throw std::runtime_error("DataProcessorAlgorithm::splitInput is not implemented");
  }

  void DataProcessorAlgorithm::forwardProperties()
  {
    throw std::runtime_error("DataProcessorAlgorithm::forwardProperties is not implemented");
  }
} // namespace Mantid
} // namespace API
