#pragma once

#include <nanovg.h>

// TODO: This doesn't feel right..
namespace bikeshed {
  const NVGcolor gray = nvgRGB(96.f, 96.f, 96.f);

  const float whiteR = 240.f;
  const float whiteG = 246.f;
  const float whiteB = 253.f;
  const NVGcolor white = nvgRGB(whiteR, whiteG, whiteB);
}
