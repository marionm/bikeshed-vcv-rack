#pragma once

#include "extern/signalsmith-stretch/signalsmith-stretch.h"

#include <rack.hpp>

#include <cstddef>

namespace bikeshed {

// Streams input into a fixed, caller-owned output buffer. Once that buffer is
// full, rendering continues into a bounded discard buffer so Signalsmith keeps
// its input history across output-buffer boundaries.
struct Timestretcher {
  static constexpr size_t maxInputSamplesPerRender = 256;
  static constexpr size_t maxOutputSamplesPerRender = 256;

  Timestretcher();
  explicit Timestretcher(float sampleRate);

  void setSampleRate(float sampleRate);
  void setOutputBuffer(float* output, size_t outputCapacity);
  void setPlaybackSpeed(float playbackSpeed);
  void setPitchCorrection(float pitchCorrection);
  void pushInput(float inputSample);

  // Call once per audio sample. Rendering occurs when enough input has
  // accumulated for a bounded input/output work unit.
  void render();
  // Render the remaining partial input block before freezing this generation.
  void finish();

  bool isOutputFull() const;
  size_t getInputLatencySampleCount() const;
  size_t getRenderedSampleCount() const;

private:
  void configure();
  void renderInputBlock(size_t inputSampleCount);

  float inputBuffer[maxInputSamplesPerRender];
  float discardOutputBuffer[maxOutputSamplesPerRender];
  size_t bufferedInputSampleCount = 0;

  float* output = nullptr;
  size_t outputCapacity = 0;
  size_t renderedOutputSampleCount = 0;
  float playbackSpeed = 1.f;
  bool acceptingInput = false;

  float sampleRate = 48000.f;
  signalsmith::stretch::SignalsmithStretch<float> stretch;
};

} // namespace bikeshed
