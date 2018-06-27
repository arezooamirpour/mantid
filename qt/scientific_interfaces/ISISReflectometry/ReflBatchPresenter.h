#ifndef MANTID_ISISREFLECTOMETRY_REFLBATCHPRESENTER_H
#define MANTID_ISISREFLECTOMETRY_REFLBATCHPRESENTER_H

#include "DllConfig.h"
#include "IReflBatchPresenter.h"
#include <memory>

namespace MantidQt {
namespace CustomInterfaces {

class IReflBatchView;
class IReflRunsTabPresenter;
class IReflEventTabPresenter;
class IReflSettingsTabPresenter;
class IReflSaveTabPresenter;

/** @class ReflBatchPresenter

ReflBatchPresenter is the concrete main window presenter implementing the
functionality defined by the interface IReflBatchPresenter.

Copyright &copy; 2011-14 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
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

File change history is stored at: <https://github.com/mantidproject/mantid>.
Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class MANTIDQT_ISISREFLECTOMETRY_DLL ReflBatchPresenter
    : public IReflBatchPresenter {
public:
  /// Constructor
  ReflBatchPresenter(IReflBatchView *view,
                     std::unique_ptr<IReflRunsTabPresenter> runsPresenter,
                     std::unique_ptr<IReflEventTabPresenter> eventPresenter,
                     std::unique_ptr<IReflSettingsTabPresenter> settingsPresenter,
                     std::unique_ptr<IReflSaveTabPresenter> savePresenter);

  /// Returns values passed for 'Transmission run(s)'
  MantidWidgets::DataProcessor::OptionsQMap
  getOptionsForAngle(int group, const double angle) const override;
  /// Whether there are per-angle transmission runs specified
  bool hasPerAngleOptions(int group) const override;
  /// Returns global options for 'CreateTransmissionWorkspaceAuto'
  MantidWidgets::DataProcessor::OptionsQMap
  getTransmissionOptions(int group) const override;
  /// Returns global options for 'ReflectometryReductionOneAuto'
  MantidWidgets::DataProcessor::OptionsQMap
  getReductionOptions(int group) const override;
  /// Returns global options for 'Stitch1DMany'
  std::string getStitchOptions() const override;
  /// Returns time-slicing values
  std::string getTimeSlicingValues(int group) const override;
  /// Returns time-slicing type
  std::string getTimeSlicingType(int group) const override;

  /// Set the instrument name
  void setInstrumentName(const std::string &instName) const override;

  /// Returns whether the Runs Tab is currently processing any runs
  bool isProcessing() const override;
  void settingsChanged() override;
  void notifyReductionPaused(int group) override;
  void notifyReductionResumed(int group) override;
  bool requestClose() const override;

  void completedGroupReductionSuccessfully(
      MantidWidgets::DataProcessor::GroupData const &group,
      std::string const &workspaceName) override;
  void completedRowReductionSuccessfully(
      MantidWidgets::DataProcessor::GroupData const &group,
      std::string const &workspaceName) override;

private:
  /// Pauses reduction in the Runs Tab
  void pauseReduction() const;
  /// Resumes reduction in the Runs Tab
  void resumeReduction() const;
  /// The view we are handling
  IReflBatchView *m_view;
  /// The presenter of tab 'Runs'
  std::unique_ptr<IReflRunsTabPresenter> m_runsPresenter;
  /// The presenter of tab 'Event Handling'
  std::unique_ptr<IReflEventTabPresenter> m_eventPresenter;
  /// The presenter of tab 'Settings'
  std::unique_ptr<IReflSettingsTabPresenter> m_settingsPresenter;
  /// The presenter of tab 'Save ASCII'
  std::unique_ptr<IReflSaveTabPresenter> m_savePresenter;
};
}
}
#endif /* MANTID_ISISREFLECTOMETRY_REFLBATCHPRESENTER_H */
