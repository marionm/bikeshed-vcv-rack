#include <rack.hpp>

#include <string>

struct LengthParamQuantity : rack::ParamQuantity {
  void setDisplayValueString(std::string string) override;
  std::string getDisplayValueString() override;
  std::string getString() override;

  int length, totalLength;
};
