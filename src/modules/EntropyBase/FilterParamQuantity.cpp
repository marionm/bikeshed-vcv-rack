#include "FilterParamQuantity.hpp"

using namespace rack;

std::string FilterParamQuantity::getString() {
  float value = getValue();

  if (value == 0.f) {
    return getLabel() + ": None";
  } else if (value > 0.f) {
    return getLabel() + string::f(" >= %.2g", value);
  } else {
    return getLabel() + string::f(" <= %.2g", value + 1.f);
  }
}

float FilterParamQuantity::getDisplayValue() {
  float value = getValue();
  return value >= 0.f ? value : (value + 1.f) * -1.f;
}

void FilterParamQuantity::setDisplayValue(float value) {
  try {
    if (value < 0.f) {
      value = -1.f - value;
    }

    if (value < -1.f) {
      value = -1.f;
    } else if (value > 1.f) {
      value = 1.f;
    }

    setValue(value);
  } catch(...) {}
}
