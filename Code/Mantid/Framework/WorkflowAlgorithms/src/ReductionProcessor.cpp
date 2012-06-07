#include "MantidWorkflowAlgorithms/ReductionProcessor.h"
#include "MantidAPI/AlgorithmProperty.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/IEventWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidAPI/FileFinder.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/FileProperty.h"
#include <stdexcept>
#ifdef MPI_BUILD
#include <boost/mpi.hpp>
#endif

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace WorkflowAlgorithms
{
  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(ReductionProcessor)

  /// Sets documentation strings for this algorithm
  void ReductionProcessor::initDocs()
  {
    this->setWikiSummary("Data processor algorithm.");
    this->setOptionalMessage("Data processor algorithm.");
  }

  void ReductionProcessor::init()
  {
    // Input data object (File or Workspace)
    declareProperty("InputData", "", "Input data, either as a file path or a workspace name");
    declareProperty("LoadAlgorithm", "LoadEventNexus");
    declareProperty("ProcessingAlgorithm", "");
    declareProperty(new WorkspaceProperty<>("OutputWorkspace","",Direction::Output));
    declareProperty(new FileProperty("OutputFile", "", FileProperty::OptionalSave, ".nxs"),
        "File path for the output nexus file");
  }

  void ReductionProcessor::exec()
  {
    // Set the data loader
    const std::string loader = getProperty("LoadAlgorithm");
    setLoadAlg(loader);

    // Load the data
    const std::string inputData = getProperty("InputData");
    Workspace_sptr inputWS = load(inputData);

    // Process the data
    std::string outputWSName = getPropertyValue("OutputWorkspace");
    const std::string procAlgName = getProperty("ProcessingAlgorithm");

    IAlgorithm_sptr procAlg = createSubAlgorithm(procAlgName);
    procAlg->setPropertyValue("InputWorkspace", inputData);
    procAlg->setAlwaysStoreInADS(true);
    procAlg->setPropertyValue("OutputWorkspace", outputWSName);
    procAlg->execute();
    Workspace_sptr outputWS = AnalysisDataService::Instance().retrieve(outputWSName);

    // Assemble
    outputWS = assemble(outputWSName, outputWSName);
    setProperty("OutputWorkspace", outputWS);

    // Save as necessary
    const std::string outputFile = getPropertyValue("OutputFile");
    saveNexus(outputWSName, outputFile);
  }

}
}
