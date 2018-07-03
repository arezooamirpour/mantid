#ifndef MANTIDQTCUSTOMINTERFACESIDA_IQTFITMODEL_H_
#define MANTIDQTCUSTOMINTERFACESIDA_IQTFITMODEL_H_

#include "IndirectFittingModel.h"

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

class DLLExport IqtFitModel : public IndirectFittingModel {
public:
  IqtFitModel();
  void setFitTypeString(const std::string &fitType);

  void setFitFunction(Mantid::API::IFunction_sptr function) override;
  bool canConstrainIntensities() const;
  bool setConstrainIntensities(bool constrain);

private:
  Mantid::API::IAlgorithm_sptr sequentialFitAlgorithm() const override;
  Mantid::API::IAlgorithm_sptr simultaneousFitAlgorithm() const override;
  std::string sequentialFitOutputName() const override;
  std::string simultaneousFitOutputName() const override;
  std::string singleFitOutputName(std::size_t index,
                                  std::size_t spectrum) const override;
  std::unordered_map<std::string, ParameterValue>
  createDefaultParameters(std::size_t index) const override;

  bool m_constrainIntensities;
  std::string m_fitType;
};

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt

#endif
