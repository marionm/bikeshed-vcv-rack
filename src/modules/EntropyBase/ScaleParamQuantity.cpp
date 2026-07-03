#include "ScaleParamQuantity.hpp"

using namespace rack;

std::string ScaleParamQuantity::getString() {
  float value = getValue();

  if (value == 0) {
    return getLabel() + ": 0 V";
  } else if (value > 0) {
    return string::f("%s: 0 V to %.2f V", getLabel(), value * 10.f);
  } else {
    return string::f("%s: %.2f V to %.2f V", getLabel(), value * 5.f, value * -5.f);
  }
}
