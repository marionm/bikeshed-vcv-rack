#include "EntropyBase.hpp"
#include "FilterParamQuantity.hpp"
#include "OffsetParamQuantity.hpp"
#include "ScaleParamQuantity.hpp"

#include <random>
#include <string>

using namespace rack;

EntropyBase::EntropyBase(int totalLength)
  : totalLength(totalLength)
{
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

  std::string clockLabel = "Clock";
  configInput(CLOCK_INPUT, clockLabel);
  configButton(CLOCK_PARAM, clockLabel);

  std::string runLabel = "Run";
  configInput(RUN_INPUT, runLabel);
  configButton(RUN_PARAM, runLabel);

  std::string resetLabel = "Reset";
  configInput(RESET_INPUT, resetLabel);
  configButton(RESET_PARAM, resetLabel);

  std::string randomLabel = "Randomize";
  configInput(RANDOM_INPUT, randomLabel);
  configButton(RANDOM_PARAM, randomLabel);

  std::string startLabel = "Start index";
  configInput(START_INPUT, startLabel);
  configParam(START_PARAM, 0, totalLength - 1.f, 0.f, startLabel);
  getParamQuantity(START_PARAM)->snapEnabled = true;
  configParam(START_CV_PARAM, -1.f, 1.f, 0.f, startLabel.append(" CV"), "%", 0, 100);
  getParamQuantity(START_CV_PARAM)->randomizeEnabled = false;

  std::string filterLabel = "Filter";
  configInput(FILTER_INPUT, filterLabel);
  configParam<FilterParamQuantity>(FILTER_PARAM, -1.f, 1.f, 0.f, filterLabel);
  configParam(FILTER_CV_PARAM, -1.f, 1.f, 0.f, filterLabel.append(" CV"), "%", 0, 100);
  getParamQuantity(FILTER_CV_PARAM)->randomizeEnabled = false;

  std::string offsetLabel = "Length";
  configInput(OFFSET_INPUT, offsetLabel);
  configParam<OffsetParamQuantity>(OFFSET_PARAM, 1.f - totalLength, totalLength - 1.f, 7.f, offsetLabel);
  getParamQuantity(OFFSET_PARAM)->snapEnabled = true;
  configParam(OFFSET_CV_PARAM, -1.f, 1.f, 0.f, offsetLabel.append(" CV"), "%", 0, 100);
  getParamQuantity(OFFSET_CV_PARAM)->randomizeEnabled = false;

  configOutput(EOS_OUTPUT, "End of sequence");
  configOutput(TRIGGER_OUTPUT, "Trigger");
  configOutput(GATE_OUTPUT, "Gate");
  configOutput(CV_OUTPUT, "CV");

  configParam<ScaleParamQuantity>(SCALE_PARAM, -1.f, 1.f, .1f, "Scale");
  getParamQuantity(SCALE_PARAM)->randomizeEnabled = false;

  maxIndex = totalLength - 1;

  randomizeValues();
}

void EntropyBase::onRandomize() {
  Module::onRandomize();
  randomizeSeed();
  randomizeValues();
}

void EntropyBase::onReset() {
  seed = 42u;
  index = 0;
  randomizeValues();
}

void EntropyBase::process(const ProcessArgs& args) {
  updateFilter();
  bool isRunning = updateRun();
  updateValues(args);

  bool isReversed = updateRange();
  updateIndex(args, isRunning, isReversed);
}

void EntropyBase::updateFilter() {
  float filter = math::clamp(
    params[FILTER_PARAM].getValue() +
    inputs[FILTER_INPUT].getVoltage() / 10.f * params[FILTER_CV_PARAM].getValue(),
    -1.f, 1.f
  );

  if (filter == 0.f) {
    minValue = 0.f;
    maxValue = 1.f;
  } else if (filter > 0.f) {
    minValue = filter;
    maxValue = 1.f;
  } else {
    minValue = 0.f;
    maxValue = 1 + filter;
  }
}

bool EntropyBase::updateRange() {
  int startIndex = params[START_PARAM].getValue();
  int cvStartIndex = (inputs[START_INPUT].getVoltage() / 10.f * params[START_CV_PARAM].getValue() * (totalLength - 1.f));
  startIndex = math::clamp(startIndex + cvStartIndex, 0, totalLength - 1);

  int offset = params[OFFSET_PARAM].getValue();
  int cvOffset = (inputs[OFFSET_INPUT].getVoltage() / 10.f * params[OFFSET_CV_PARAM].getValue() * (totalLength - 1.f));
  offset = math::clamp(offset + cvOffset, 1 - totalLength, totalLength - 1);

  bool isReversed = offset < 0;
  if (isReversed) {
    minIndex = clampRangeIndex(startIndex + offset);
    maxIndex = clampRangeIndex(startIndex);
  } else {
    minIndex = clampRangeIndex(startIndex);
    maxIndex = clampRangeIndex(startIndex + offset);
  }

  clampIndex(isReversed);

  return isReversed;
}

bool EntropyBase::updateRun() {
  if (runTrigger.process(inputs[RUN_INPUT].getVoltage())) {
    params[RUN_PARAM].setValue(params[RUN_PARAM].getValue() >= 0.5f ? 0.f : 1.f);
  }

  bool isRunning = params[RUN_PARAM].getValue() >= 0.5f;
  lights[RUN_LIGHT].setBrightness(isRunning ? 1.f : 0.f);

  return isRunning;
}

void EntropyBase::updateValues(const ProcessArgs& args) {
  if (
    randomButtonTrigger.process(params[RANDOM_PARAM].getValue()) ||
    randomTrigger.process(inputs[RANDOM_INPUT].getVoltage())
  ) {
    randomizeSeed();
    randomizeValues();
    randomPulse.trigger(1e-3f);
  }

  lights[RANDOM_LIGHT].setSmoothBrightness(randomPulse.process(args.sampleTime), args.sampleTime);
}

void EntropyBase::updateIndex(const ProcessArgs& args, bool isRunning, bool isReversed) {
  bool didStep = false;
  if (isRunning && (
    clockButtonTrigger.process(params[CLOCK_PARAM].getValue()) ||
    clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())
  )) {
    index += isReversed ? -1 : 1;
    didStep = true;
    clockPulse.trigger(1e-3f);

    if (clampIndex(isReversed)) {
      eosPulse.trigger(1e-3f);
    }

    if (getValue() > 0.f) {
      triggerPulse.trigger(1e-3f);
    }
  }

  lights[CLOCK_LIGHT].setSmoothBrightness(clockPulse.process(args.sampleTime), args.sampleTime);

  if (
    resetButtonTrigger.process(params[RESET_PARAM].getValue()) ||
    resetTrigger.process(inputs[RESET_INPUT].getVoltage())
  ) {
    index = isReversed ? maxIndex : minIndex;
    resetPulse.trigger(1e-3f);
  }

  lights[RESET_LIGHT].setSmoothBrightness(resetPulse.process(args.sampleTime), args.sampleTime);

  bool hitEos = eosPulse.process(args.sampleTime);
  lights[EOS_LIGHT].setSmoothBrightness(hitEos, args.sampleTime);
  outputs[EOS_OUTPUT].setVoltage(hitEos && isRunning ? 10.f : 0.f);

  bool hitTrigger = triggerPulse.process(args.sampleTime);
  lights[TRIGGER_LIGHT].setSmoothBrightness(hitTrigger, args.sampleTime);
  outputs[TRIGGER_OUTPUT].setVoltage(hitTrigger && isRunning ? 10.f : 0.f);

  float value = getValue();
  outputs[CV_OUTPUT].setVoltage(isRunning ? scaleValue(value) : 0);
  updateGateOutput(args, value, didStep, isRunning);
}

void EntropyBase::updateGateOutput(const ProcessArgs& args, float value, bool didStep, bool isRunning) {
  timeSinceLastClock += args.sampleTime;

  if (isGateActive) {
    gateTime += args.sampleTime;
    if (gateTime >= maxGateTime) {
      isGateActive = false;
    }
  }

  if (didStep) {
    // Can't output gates until we have at least one clock trigger for duration calculations
    if (hasStepped) {
      gateTime = 0.f;
      maxGateTime = timeSinceLastClock * value;
      isGateActive = maxGateTime > 0.f;
    } else {
      hasStepped = true;
    }

    timeSinceLastClock = 0.f;
  }

  lights[GATE_LIGHT].setSmoothBrightness(isGateActive && isRunning, args.sampleTime);
  outputs[GATE_OUTPUT].setVoltage(isGateActive && isRunning ? 10.f : 0.f);
}

float EntropyBase::getValue() {
  float value = values[index];

  if (minValue <= value && value <= maxValue) {
    return value;
  } else {
    return 0.f;
  }
}

float EntropyBase::scaleValue(float value) {
  float scale = params[SCALE_PARAM].getValue() * 10.f;
  if (scale >= 0) {
    return value * scale;
  } else {
    return (value - .5f) * -scale;
  }
}

bool EntropyBase::isInRange(int index) const {
  if (minIndex <= maxIndex) {
    return index >= minIndex && index <= maxIndex;
  } else {
    return index >= minIndex || index <= maxIndex;
  }
}

bool EntropyBase::clampIndex(bool isReversed) {
  if (!isInRange(index)) {
    index = isReversed ? maxIndex : minIndex;
    return true;
  } else {
    return false;
  }
}

int EntropyBase::clampRangeIndex(int index) {
  if (index < 0) {
    return index + totalLength;
  } else if (index >= totalLength) {
    return index - totalLength;
  } else {
    return index;
  }
}

void EntropyBase::randomizeSeed() {
  std::random_device rd;
  seed = (uint32_t(rd()) << 16) ^ uint32_t(rd());
}

void EntropyBase::randomizeValues() {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> distribution(0.f, 1.f);

  values.clear();
  values.reserve(totalLength);
  for (int i = 0; i < (int)totalLength; ++i) {
    values.push_back(distribution(rng));
  }
}

json_t* EntropyBase::dataToJson() {
  json_t* root = json_object();

  json_t* valuesJson = json_array();
  for (auto& value : values) {
    json_array_append_new(valuesJson, json_real(value));
  }
  json_object_set_new(root, "values", valuesJson);
  json_object_set_new(root, "seed", json_integer(seed));
  json_object_set_new(root, "index", json_integer(index));

  return root;
}

void EntropyBase::dataFromJson(json_t* root) {
  if (json_t* valuesJson = json_object_get(root, "values")) {
    if (json_is_array(valuesJson)) {
      values.clear();
      size_t index;
      json_t* valueJson;
      json_array_foreach(valuesJson, index, valueJson) {
        if (json_is_number(valueJson)) {
          values.push_back((float)json_number_value(valueJson));
        }
      }
    }
  }

  if (json_t* seedJson = json_object_get(root, "seed")) {
    if (json_is_integer(seedJson)) {
      seed = (uint32_t)json_integer_value(seedJson);
    }
  }

  if (json_t* currentIndexJson = json_object_get(root, "index")) {
    if (json_is_integer(currentIndexJson)) {
      index = (int)json_integer_value(currentIndexJson);
    }
  }
}
