#include "Bikeshed.hpp"
#include "components/Knob.hpp"

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
    configParam(SPEED_PARAM, 0.f, 16.f, 2.f, "Playback speed");

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
    recordingBuffer.push(audioIn);

    clockDivider.setDivision(params[DELAY_PARAM].getValue());
    bool clockElapsed = clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()) && clockDivider.process();
    bool timeElapsed = timer.process(args.sampleTime) >= params[DELAY_PARAM].getValue();
    bool trigger = getInput(CLOCK_INPUT).isConnected() ? clockElapsed : timeElapsed;
    lights[TRIGGER_DEBUG_LIGHT].setSmoothBrightness(trigger, args.sampleTime);
    if (trigger) {
      clockDivider.reset();
      timer.reset();
      i = 0;
      size = recordingBuffer.size();
      recordingBuffer.shiftBuffer(playbackBuffer, size);
      recordingBuffer.clear();
      playing = true;
    }
    lights[PLAYING_DEBUG_LIGHT].setSmoothBrightness(playing, args.sampleTime);

    bool stopped = false;
    if (i >= size) {
      i = 0;
      playing = false;
      stopped = true;
    }
    lights[STOPPED_DEBUG_LIGHT].setSmoothBrightness(stopped, args.sampleTime);

    if (i2++ % 4410 == 0) {
      DEBUG("i: %i, size: %i, playing %i", i, size, playing);
    }

    float dryOut, wetOut;
    if (playing) {
      dryOut = params[MASK_PARAM].getValue() >= .5f ? 0.f : audioIn;
      wetOut = playbackBuffer[i];
      i++;
    } else {
      dryOut = audioIn;
      wetOut = 0.f;
    }

    outputs[MIX_OUTPUT].setVoltage(crossfade(dryOut, wetOut, params[MIX_PARAM].getValue()));
    outputs[WET_OUTPUT].setVoltage(playing ? wetOut : 0.f);
  }

  bool playing = false;
  int i = 0;
  int i2 = 0;
  int size = 0;
  static constexpr int bufferSize = 1 << 21;

  dsp::SchmittTrigger clockTrigger;
  dsp::ClockDivider clockDivider;
  dsp::Timer timer;

  dsp::RingBuffer<float, bufferSize> recordingBuffer;
  float playbackBuffer[bufferSize];
};

struct BreakpointWidget : app::ModuleWidget {
  BreakpointWidget(Module* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/modules/EntropyPool.svg")));

    float x = 62.74;
    float y = 82;
    float d = 16;
    addParam(createParamCentered<MediumKnob<false, true>>(mm2px(Vec(x + d * 0, y)), module, Breakpoint::DELAY_PARAM));
    addParam(createParamCentered<MediumKnob<>>(mm2px(Vec(x + d * 1, y)), module, Breakpoint::MIX_PARAM));
    addParam(createParamCentered<MediumKnob<>>(mm2px(Vec(x + d * 2, y)), module, Breakpoint::SPEED_PARAM));
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
