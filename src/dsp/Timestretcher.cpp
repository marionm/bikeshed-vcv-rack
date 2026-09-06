#include "Timestretcher.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

using namespace bikeshed;

Timestretcher::Timestretcher()
  : Timestretcher(48000.f)
{}

Timestretcher::Timestretcher(float sampleRate)
  : sampleRate(sampleRate) {
  configure();
}

void Timestretcher::configure() {
  stretch.presetDefault(1, sampleRate);
  stretch.setTransposeFactor(1.f);
  stretch.reset();
}

void Timestretcher::setSampleRate(float sampleRate) {
  this->sampleRate = sampleRate;
  configure();
  bufferedInputSampleCount = 0;
  acceptingInput = false;
}

void Timestretcher::setOutputBuffer(float* output, size_t outputCapacity) {
  this->output = output;
  this->outputCapacity = outputCapacity;
  renderedOutputSampleCount = 0;
  acceptingInput = output && outputCapacity > 0;
}

void Timestretcher::setPlaybackSpeed(float playbackSpeed) {
  this->playbackSpeed = std::max(playbackSpeed, 0.001f);
}

void Timestretcher::setPitchCorrection(float pitchCorrection) {
  stretch.setTransposeFactor(pitchCorrection);
}

void Timestretcher::pushInput(float inputSample) {
  if (!acceptingInput || bufferedInputSampleCount >= maxInputSamplesPerRender) {
    return;
  }
  inputBuffer[bufferedInputSampleCount++] = inputSample;
}

void Timestretcher::render() {
  if (!acceptingInput || bufferedInputSampleCount == 0) {
    return;
  }

  size_t inputSamplesForOutputBlock = std::max<size_t>(1,
    static_cast<size_t>(std::floor(maxOutputSamplesPerRender * playbackSpeed))
  );
  inputSamplesForOutputBlock = std::min(inputSamplesForOutputBlock, maxInputSamplesPerRender);
  if (bufferedInputSampleCount < inputSamplesForOutputBlock) {
    return;
  }

  renderInputBlock(inputSamplesForOutputBlock);
}

void Timestretcher::finish() {
  if (!acceptingInput || bufferedInputSampleCount == 0) {
    return;
  }
  renderInputBlock(bufferedInputSampleCount);
}

void Timestretcher::renderInputBlock(size_t inputSampleCount) {
  size_t remainingOutputCapacity = outputCapacity - renderedOutputSampleCount;
  if (remainingOutputCapacity > 0) {
    size_t inputSamplesForRemainingOutput = std::max<size_t>(1,
      static_cast<size_t>(std::floor(remainingOutputCapacity * playbackSpeed))
    );
    inputSampleCount = std::min(inputSampleCount, inputSamplesForRemainingOutput);
  }

  size_t outputSampleCount = std::max<size_t>(1,
    static_cast<size_t>(std::round(inputSampleCount / playbackSpeed))
  );
  float* outputBlock = discardOutputBuffer;
  if (remainingOutputCapacity > 0) {
    outputSampleCount = std::min(outputSampleCount, remainingOutputCapacity);
    outputBlock = output + renderedOutputSampleCount;
  }

  float* inputBlock = inputBuffer;
  stretch.process(&inputBlock, static_cast<int>(inputSampleCount), &outputBlock,
    static_cast<int>(outputSampleCount));
  if (remainingOutputCapacity > 0) {
    renderedOutputSampleCount += outputSampleCount;
  }

  size_t remainingInputSampleCount = bufferedInputSampleCount - inputSampleCount;
  if (remainingInputSampleCount > 0) {
    std::memmove(inputBuffer, inputBuffer + inputSampleCount,
      remainingInputSampleCount * sizeof(float));
  }
  bufferedInputSampleCount = remainingInputSampleCount;
}

bool Timestretcher::isOutputFull() const {
  return renderedOutputSampleCount == outputCapacity;
}

size_t Timestretcher::getInputLatencySampleCount() const {
  return static_cast<size_t>(stretch.inputLatency());
}

size_t Timestretcher::getRenderedSampleCount() const {
  return renderedOutputSampleCount;
}
