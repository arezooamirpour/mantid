#ifndef MANTID_VATES_PeaksFilter_H
#define MANTID_VATES_PeaksFilter_H

#include "MantidAPI/IPeaksWorkspace.h"
#include "MantidDataObjects/PeakShapeBase.h"
#include "MantidKernel/System.h"
#include "MantidKernel/V3D.h"
#include "MantidVatesAPI/ProgressAction.h"
#include "vtkSmartPointer.h"
#include <string>

class vtkUnstructuredGrid;

namespace Mantid
{
namespace VATES
{

  /**
    Class that selects only those data points which lie within the geometry of a peak. 
    
    @date 17/02/2015

    Copyright &copy; 2015 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge National Laboratory & European Spallation Source

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

  class DLLExport vtkDataSetToPeaksFilteredDataSet
  {
    public:
      vtkDataSetToPeaksFilteredDataSet(
          vtkSmartPointer<vtkUnstructuredGrid> input,
          vtkSmartPointer<vtkUnstructuredGrid> output);
      virtual ~vtkDataSetToPeaksFilteredDataSet();
      /// Set the name of the peaks workspace
      void initialize(
          const std::vector<Mantid::API::IPeaksWorkspace_sptr> &peaksWorkspaces,
          double radiusNoShape, Geometry::PeakShape::RadiusType radiusType,
          int coordinateSystem);
      /// Apply the peak filtering
      void execute(ProgressAction& progressUpdating);
      /// Get radius of no shape
      double getRadiusNoShape();
      /// Get radius factor
      double getRadiusFactor();
    private:
      vtkDataSetToPeaksFilteredDataSet& operator=(const vtkDataSetToPeaksFilteredDataSet& other);
      std::vector<std::pair<Mantid::Kernel::V3D, double>>
      getPeaksInfo(const std::vector<Mantid::API::IPeaksWorkspace_sptr>
                       &peaksWorkspaces);
      void addSinglePeak(Mantid::Geometry::IPeak* peak, const Mantid::Kernel::SpecialCoordinateSystem coordinateSystem, std::vector<std::pair<Mantid::Kernel::V3D, double>>& peaksInfo);
      vtkSmartPointer<vtkUnstructuredGrid> m_inputData; ///< Data to peak filter
      vtkSmartPointer<vtkUnstructuredGrid> m_outputData; ///< Peak filtered data
      std::vector<Mantid::API::IPeaksWorkspace_sptr> m_peaksWorkspaces; ///< A list of peaks workspace names.
      bool m_isInitialised; ///<Flag if the filter is initialized
      double m_radiusNoShape; ///< The radius for peaks with no peak shape.
      Geometry::PeakShape::RadiusType m_radiusType;
      double m_radiusFactor;///< By how much we want to trim the data set.
      double m_defaultRadius; ///< A default radius.
      int m_coordinateSystem;///< A coordinate system.
  };
}
}
#endif
