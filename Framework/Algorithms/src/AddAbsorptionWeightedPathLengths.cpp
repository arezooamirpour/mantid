// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidAlgorithms/AddAbsorptionWeightedPathLengths.h"
#include "MantidAlgorithms/MCAbsorptionWeightedPathStrategy.h"
#include "MantidAlgorithms/SampleCorrections/RectangularBeamProfile.h"
#include "MantidAPI/ExperimentInfo.h"
#include "MantidAPI/Sample.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidGeometry/Instrument.h"
#include "MantidGeometry/Instrument/ReferenceFrame.h"
#include "MantidKernel/BoundedValidator.h"

using namespace Mantid::API;
using namespace Mantid::Geometry;
using namespace Mantid::Kernel;

namespace Mantid {
namespace Algorithms {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(AddAbsorptionWeightedPathLengths)

//----------------------------------------------------------------------------------------------

namespace {

constexpr int DEFAULT_NEVENTS = 1000;
constexpr int DEFAULT_SEED = 123456789;
} // namespace

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
 */
void AddAbsorptionWeightedPathLengths::init() {
  declareProperty(
      std::make_unique<WorkspaceProperty<>>("InputWorkspace", "",
                                            Direction::InOut),
      "An input/output workspace that the path distances will be added to.");
  auto positiveInt = std::make_shared<Kernel::BoundedValidator<int>>();
  positiveInt->setLower(1);
  declareProperty(
      "EventsPerPoint", DEFAULT_NEVENTS, positiveInt,
      "The number of \"neutron\" events to generate per simulated point");
  declareProperty("SeedValue", DEFAULT_SEED, positiveInt,
                  "Seed the random number generator with this value");
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
 */
void AddAbsorptionWeightedPathLengths::exec() {

  const MatrixWorkspace_sptr inputWS = getProperty("InputWorkspace");
  const int nevents = getProperty("EventsPerPoint");
  const int maxScatterPtAttempts = getProperty("MaxScatterPtAttempts");

  auto instrument = inputWS->getInstrument();
  auto beamProfile = createBeamProfile(*instrument, inputWS->sample());

  // Configure strategy
  MCAbsorptionWeightedPathStrategy strategy(
      *beamProfile, inputWS->sample(), nevents, maxScatterPtAttempts, g_log);
}

/**
 * Create the beam profile. Currently only supports Rectangular. The dimensions
 * are either specified by those provided by `SetBeam` algorithm or default
 * to the width and height of the samples bounding box
 * @param instrument A reference to the instrument object
 * @param sample A reference to the sample object
 * @return A new IBeamProfile object
 */
std::unique_ptr<IBeamProfile>
AddAbsorptionWeightedPathLengths::createBeamProfile(
    const Instrument &instrument, const Sample &sample) const {
  const auto frame = instrument.getReferenceFrame();
  const auto source = instrument.getSource();

  auto beamWidthParam = source->getNumberParameter("beam-width");
  auto beamHeightParam = source->getNumberParameter("beam-height");
  double beamWidth(-1.0), beamHeight(-1.0);
  if (beamWidthParam.size() == 1 && beamHeightParam.size() == 1) {
    beamWidth = beamWidthParam[0];
    beamHeight = beamHeightParam[0];
  } else {
    const auto bbox = sample.getShape().getBoundingBox().width();
    beamWidth = bbox[frame->pointingHorizontal()];
    beamHeight = bbox[frame->pointingUp()];
  }
  return std::make_unique<RectangularBeamProfile>(*frame, source->getPos(),
                                                  beamWidth, beamHeight);
}

} // namespace Algorithms
} // namespace Mantid