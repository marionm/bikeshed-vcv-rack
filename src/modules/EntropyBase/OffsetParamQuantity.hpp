#include <rack.hpp>

#include <string>

struct OffsetParamQuantity : rack::ParamQuantity {
  std::string getString() override;
  float getDisplayValue() override;
  void setDisplayValue(float value) override;
};
