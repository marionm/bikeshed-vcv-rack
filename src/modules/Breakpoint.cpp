#include "Bikeshed.hpp"
#include "components/Knob.hpp"
#include "dsp/Timestretcher.hpp"

#include <algorithm>
#include <cmath>
#include <math.hpp>

using namespace bikeshed;
using namespace rack;

struct Breakpoint : Module {
  enum ParamId {
    DELAY_PARAM,
    MASK_PARAM,
    MIX_PARAM,
    SPEED_PARAM,
    SPEED_CV_PARAM,
    NUM_PARAMS
  };

  enum InputId {
    AUDIO_INPUT,
    CLOCK_INPUT,
    SPEED_CV_INPUT,
    NUM_INPUTS
  };

  enum OutputId {
    MIX_OUTPUT,
    WET_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightId {
    MASK_LIGHT,

    TRIGGER_DEBUG_LIGHT,
    STOPPED_DEBUG_LIGHT,
    PLAYING_DEBUG_LIGHT,
    NUM_LIGHTS
  };

  Breakpoint() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // TODO: Sync input
    configInput(AUDIO_INPUT, "Audio");
    configInput(CLOCK_INPUT, "Clock");

    configOutput(MIX_OUTPUT, "Mix");
    configOutput(WET_OUTPUT, "Wet");

    // TODO: Snapping, tooltip units, depending on clock input connectedness
    configParam(DELAY_PARAM, 0.f, 8.f, 1.f, "Delay");
    configParam(MIX_PARAM, 0.f, 1.f, .5f, "Mix");
    // TODO: Snap to ints, and fractions? But still allow smooth? Possible?
    configParam(SPEED_PARAM, 0.125f, 16.f, 2.f, "Playback speed");

    // TODO: More CV inputs - delay
    std::string speedCvLabel = "Playback speed CV";
    configInput(SPEED_CV_INPUT, speedCvLabel);
    configParam(SPEED_CV_PARAM, -1.f, 1.f, 0.f, speedCvLabel);

    configButton(MASK_PARAM, "Mask dry output");
  }

  void process(const ProcessArgs& args) override {
    bool masked = params[MASK_PARAM].getValue() >= .5f;
    lights[MASK_LIGHT].setBrightness(masked ? 1.f : 0.f);

    float audioIn = inputs[AUDIO_INPUT].getVoltage();
    if (!isCapturing) {
      startCapture();
    }
    if (isCapturing) {
      timestretcher.pushInput(audioIn);
      timestretcher.render();
      advancePendingOutputSwitch();
    }

    clockDivider.setDivision(params[DELAY_PARAM].getValue());
    bool clockElapsed = clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()) && clockDivider.process();
    bool timeElapsed = timer.process(args.sampleTime) >= params[DELAY_PARAM].getValue();
    bool trigger = getInput(CLOCK_INPUT).isConnected() ? clockElapsed : timeElapsed;
    lights[TRIGGER_DEBUG_LIGHT].setSmoothBrightness(trigger, args.sampleTime);
    if (trigger) {
      clockDivider.reset();
      timer.reset();
      if (isCapturing && pendingOutputSlot < 0) {
        finishCapture();
      }
    }

    float wetOut = readWet();
    bool wetActive = isWetActive();
    float dryTarget = (wetActive && masked) ? 0.f : 1.f;
    float dryStep = args.sampleTime / dryFadeTime;
    dryGain += std::max(-dryStep, std::min(dryStep, dryTarget - dryGain));

    lights[PLAYING_DEBUG_LIGHT].setSmoothBrightness(wetActive, args.sampleTime);
    lights[STOPPED_DEBUG_LIGHT].setSmoothBrightness(!wetActive, args.sampleTime);

    float dryOut = audioIn * dryGain;

    outputs[MIX_OUTPUT].setVoltage(crossfade(dryOut, wetOut, params[MIX_PARAM].getValue()));
    outputs[WET_OUTPUT].setVoltage(wetOut);
  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override {
    timestretcher.setSampleRate(e.sampleRate);
    // The active playback generation remains valid. Start a new capture with
    // a stretcher configured for the new rate on the next sample.
    isCapturing = false;
    pendingOutputSlot = -1;
    renderSlot = playbackSlot >= 0 ? 1 - playbackSlot : 0;
  }

  float getLatchedPlaybackSpeed() {
    float speedCv = inputs[SPEED_CV_INPUT].getVoltage() * params[SPEED_CV_PARAM].getValue();
    return clamp(params[SPEED_PARAM].getValue() + speedCv, 0.125f, 16.f);
  }

  void startCapture() {
    playbackSizes[renderSlot] = 0;
    timestretcher.setPlaybackSpeed(getLatchedPlaybackSpeed());
    timestretcher.setOutputBuffer(playbackBuffers[renderSlot], bufferSize);
    isCapturing = true;
  }

  void finishCapture() {
    timestretcher.finish();
    playbackSizes[renderSlot] = timestretcher.getRenderedSampleCount();
    if (playbackSizes[renderSlot] == 0) {
      return;
    }

    fadeSlot = playbackSlot;
    fadeIndex = playbackIndex;
    fadeSampleCount = (fadeSlot >= 0 && fadeIndex < playbackSizes[fadeSlot])
      ? std::min<size_t>(transitionSamples, playbackSizes[fadeSlot] - fadeIndex)
      : 0;
    playbackSlot = renderSlot;
    playbackIndex = 0;
    transitionRemaining = static_cast<int>(fadeSampleCount);
    pendingOutputSlot = 1 - renderSlot;
    inputSamplesUntilOutputSwitch = timestretcher.getInputLatencySampleCount();
    // The newly latched rate applies to the stream immediately. During the
    // pending window its output extends the old playback buffer; after the
    // switch it fills the new logical capture buffer.
    timestretcher.setPlaybackSpeed(getLatchedPlaybackSpeed());
    if (inputSamplesUntilOutputSwitch == 0) {
      switchOutputBuffer();
    }
  }

  void advancePendingOutputSwitch() {
    if (pendingOutputSlot < 0 || inputSamplesUntilOutputSwitch == 0) {
      return;
    }
    if (--inputSamplesUntilOutputSwitch == 0) {
      switchOutputBuffer();
    }
  }

  void switchOutputBuffer() {
    // Keep a render block wholly within one logical capture generation.
    timestretcher.finish();
    playbackSizes[renderSlot] = timestretcher.getRenderedSampleCount();
    renderSlot = pendingOutputSlot;
    pendingOutputSlot = -1;
    playbackSizes[renderSlot] = 0;
    timestretcher.setOutputBuffer(playbackBuffers[renderSlot], bufferSize);
  }

  size_t getAvailableSampleCount(int slot) const {
    if (slot == renderSlot) {
      return timestretcher.getRenderedSampleCount();
    }
    return playbackSizes[slot];
  }

  float readSlot(int slot, size_t& index) {
    if (slot < 0 || index >= getAvailableSampleCount(slot)) {
      return 0.f;
    }
    return playbackBuffers[slot][index++];
  }

  float readFadeSample() {
    if (fadeSlot < 0 || transitionRemaining <= 0) {
      return 0.f;
    }
    return playbackBuffers[fadeSlot][fadeIndex++];
  }

  bool isWetActive() const {
    return (playbackSlot >= 0 && playbackIndex < getAvailableSampleCount(playbackSlot)) ||
      transitionRemaining > 0;
  }

  float readWet() {
    float playback = readSlot(playbackSlot, playbackIndex);
    if (transitionRemaining > 0) {
      float previous = readFadeSample();
      float t = 1.f - static_cast<float>(transitionRemaining) / fadeSampleCount;
      float result = std::cos(0.5f * M_PI * t) * previous + std::sin(0.5f * M_PI * t) * playback;
      if (--transitionRemaining == 0) {
        fadeSlot = -1;
        fadeIndex = 0;
        fadeSampleCount = 0;
      }
      return result;
    }
    return playback;
  }

  // Supports just under 11 seconds at 192 kHz. Two fixed output generations
  // reserve 16 MiB; the outgoing playback tail is faded inline while its slot
  // is reused from the beginning for the next capture.
  static constexpr int bufferSize = 1 << 21;
  static constexpr int outputBufferCount = 2;
  static constexpr int transitionSamples = 256;
  static constexpr float dryFadeTime = 0.005f;
  float playbackBuffers[outputBufferCount][bufferSize];
  size_t playbackSizes[outputBufferCount] = {};
  int renderSlot = 0;
  int playbackSlot = -1;
  int fadeSlot = -1;
  size_t playbackIndex = 0;
  size_t fadeIndex = 0;
  size_t fadeSampleCount = 0;
  int transitionRemaining = 0;
  int pendingOutputSlot = -1;
  size_t inputSamplesUntilOutputSwitch = 0;
  bool isCapturing = false;
  float dryGain = 1.f;
  Timestretcher timestretcher;

  dsp::SchmittTrigger clockTrigger;
  dsp::ClockDivider clockDivider;
  dsp::Timer timer;
};

struct BreakpointWidget : app::ModuleWidget {
  BreakpointWidget(Module* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/modules/EntropyPool.svg")));

    float x = 62.74;
    float y = 82;
    float d = 16;
    addParam(createParamCentered<MediumKnob<false, true>>(mm2px(Vec(x + d * 0, y)), module, Breakpoint::DELAY_PARAM));
    addParam(createParamCentered<MediumKnob<false, true>>(mm2px(Vec(x + d * 1, y)), module, Breakpoint::MIX_PARAM));
    addParam(createParamCentered<MediumKnob<false, true>>(mm2px(Vec(x + d * 2, y)), module, Breakpoint::SPEED_PARAM));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(x + d * 3, y)), module, Breakpoint::MASK_PARAM, Breakpoint::MASK_LIGHT));

    x = 67.74;
    y = 94.5;
    d = 11;
    addParam(createParamCentered<SmallKnob<>>(mm2px(Vec(x + d * 0, y)), module, Breakpoint::SPEED_CV_PARAM));

    x = 111.74;
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(x + d * 0, y)), module, Breakpoint::TRIGGER_DEBUG_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(x + d * 1, y)), module, Breakpoint::PLAYING_DEBUG_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(x + d * 2, y)), module, Breakpoint::STOPPED_DEBUG_LIGHT));

    x = 12.74;
    y = 113.115; // Lines up with many VCV plugins
    d = 11;
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 0, y)), module, Breakpoint::AUDIO_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 1, y)), module, Breakpoint::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(x + d * 2, y)), module, Breakpoint::SPEED_CV_INPUT));

    x = 111.74;
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x + d * 0, y)), module, Breakpoint::MIX_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x + d * 1, y)), module, Breakpoint::WET_OUTPUT));

  }
};

Model* breakpointModel = createModel<Breakpoint, BreakpointWidget>("Breakpoint");
