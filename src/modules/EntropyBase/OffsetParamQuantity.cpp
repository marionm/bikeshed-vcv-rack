#include "OffsetParamQuantity.hpp"

using namespace rack;

std::string OffsetParamQuantity::getString() {
  float length = getDisplayValue();

  if (length >= 0.f) {
    return string::f("%s: %g", getLabel(), length);
  } else {
    return string::f("%s: %g (reversed)", getLabel(), -length);
  }
}

float OffsetParamQuantity::getDisplayValue() {
  float offset = getValue();

  if (offset >= 0.f) {
    return offset + 1.f;
  } else {
    return offset - 1.f;
  }
}

void OffsetParamQuantity::setDisplayValue(float length) {
  if (length >= 1.f) {
    length -= 1.f;
  } else if (length <= -1.f) {
    length += 1.f;
  }

  setValue(length);
}
