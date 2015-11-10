#ifndef MANTID_ALGORITHMS_CALCULATEDIFC_H_
#define MANTID_ALGORITHMS_CALCULATEDIFC_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"
#include "MantidDataObjects/OffsetsWorkspace.h"
#include "MantidDataObjects/SpecialWorkspace2D.h"
#include "MantidGeometry/IDetector.h"

using Mantid::API::MatrixWorkspace;
using Mantid::API::WorkspaceProperty;
using Mantid::DataObjects::OffsetsWorkspace;
using Mantid::DataObjects::OffsetsWorkspace_sptr;
using Mantid::DataObjects::SpecialWorkspace2D;
using Mantid::DataObjects::SpecialWorkspace2D_sptr;
using Mantid::Geometry::Instrument_const_sptr;
using Mantid::Kernel::Direction;

namespace Mantid {
namespace Algorithms {

/** CalculateDIFC : Calculate the DIFC for every pixel

  Copyright &copy; 2015 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
  National Laboratory & European Spallation Source

  This file is part of Mantid.

  Mantid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Mantid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport CalculateDIFC : public API::Algorithm {
public:
  CalculateDIFC();
  virtual ~CalculateDIFC();

  virtual const std::string name() const;
  virtual int version() const;
  virtual const std::string category() const;
  virtual const std::string summary() const;

private:
  void init();
  void exec();

  /// Calculate the DIFC for every pixel
  void calculate(API::Progress &progress, API::MatrixWorkspace_sptr &outputWs, DataObjects::OffsetsWorkspace_sptr &offsetsWs,
	  double l1, double beamlineNorm, Kernel::V3D &beamline, Kernel::V3D &samplePos, detid2det_map &allDetectors);
};

} // namespace Algorithms
} // namespace Mantid

#endif /* MANTID_ALGORITHMS_CALCULATEDIFC_H_ */
