#include <rack.hpp>

#include <string>

struct FilterParamQuantity : rack::ParamQuantity {
  std::string getString() override;
  float getDisplayValue() override;
  void setDisplayValue(float value) override;
};

