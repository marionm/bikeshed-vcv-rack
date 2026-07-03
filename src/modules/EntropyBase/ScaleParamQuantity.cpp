#include "ScaleParamQuantity.hpp"

using namespace rack;

std::string ScaleParamQuantity::getString() {
  float value = getValue();

  if (value == 0) {
    return getLabel() + ": 0 V";
  } else if (value > 0) {
    return string::f("%s: 0 V to %.3g V", getLabel(), value * 10.f);
  } else {
    return string::f("%s: %.3g V to %.3g V", getLabel(), value * 5.f, value * -5.f);
  }
}

float ScaleParamQuantity::getDisplayValue() {
  float value = getValue();
  return value * (value >= 0 ? 10.f : 5.f);
}

void ScaleParamQuantity::setDisplayValue(float value) {
  setValue(value / (value >= 0 ? 10.f : 5.f));
}
