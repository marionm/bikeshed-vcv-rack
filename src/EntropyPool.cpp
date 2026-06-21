#include "plugin.hpp"
#include "IntegrationsModal.hpp"

using namespace rack;

struct EntropyPool : Module {
  enum ParamId {
    RUN_PARAM,
    RESET_PARAM,
    RANDOM_PARAM,
    START_PARAM,
    LENGTH_PARAM,
    PING_PONG_PARAM,
    FILTER_PARAM,
    SCALE_PARAM,
    NUM_PARAMS
  };

  enum InputId {
    CLOCK_INPUT,
    RUN_INPUT,
    RESET_INPUT,
    RANDOM_INPUT,
    START_INPUT,
    LENGTH_INPUT,
    PING_PONG_INPUT,
    FILTER_INPUT,
    SCALE_INPUT,
    NUM_INPUTS
  };

  enum OutputId {
    CV_OUTPUT,
    TRIGGER_OUTPUT,
    END_OF_SEQUENCE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightId {
    CLOCK_LIGHT,
    RUN_LIGHT,
    RESET_LIGHT,
    RANDOM_LIGHT,
    PING_PONG_LIGHT,
    NUM_LIGHTS
  };

  std::vector<float> values;

  EntropyPool() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(RUN_PARAM, 0.f, 1.f, 0.f, "Run");
    configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
    configParam(RANDOM_PARAM, 0.f, 1.f, 0.f, "Randomize");
    configParam(START_PARAM, 0.f, 1.f, 0.f, "Start offset");
    configParam(LENGTH_PARAM, 0.f, 1.f, 0.f, "Sequence length");
    configParam(PING_PONG_PARAM, 0.f, 1.f, 0.f, "Ping pong");
    configParam(FILTER_PARAM, 0.f, 1.f, 0.f, "Filter");
    configParam(SCALE_PARAM, 0.f, 1.f, 0.f, "Scale");

    configInput(CLOCK_INPUT, "Clock");
    configInput(RUN_INPUT, "Run");
    configInput(RESET_INPUT, "Reset");
    configInput(RANDOM_INPUT, "Randomize");
    configInput(START_INPUT, "Start offset");
    configInput(LENGTH_INPUT, "Sequence length");
    configInput(PING_PONG_INPUT, "Ping pong");
    configInput(FILTER_INPUT, "Filter");
    configInput(SCALE_INPUT, "Scale");

    configOutput(CV_OUTPUT, "CV");
    configOutput(TRIGGER_OUTPUT, "Trigger");
    configOutput(END_OF_SEQUENCE_OUTPUT, "End of sequence");

  }

  void process(const ProcessArgs&) override {
    // TODO
  }

  json_t* dataToJson() override {
    json_t* root = json_object();

    json_t* valuesJson = json_array();
    for (auto& value : values) {
      json_array_append_new(valuesJson, json_integer(value));
    }
    json_object_set_new(root, "values", valuesJson);

    return root;
  }

  void dataFromJson(json_t* root) override {
    if (json_t* valuesJson = json_object_get(root, "values")) {
      if (json_is_array(valuesJson)) {
        values.clear();
        size_t index;
        json_t* valueJson;
        json_array_foreach(valuesJson, index, valueJson) {
          if (json_is_integer(valueJson)) {
            values.push_back(json_integer_value(valueJson));
          }
        }
      }
    }
  }
};

struct Grid : Widget {
  EntropyPool* module = nullptr;

  void draw(const DrawArgs &args) override {
    if (!module) {
      // TODO: Draw random sample
      return;
    }

    try {
      DEBUG("size: %i", (int)module->values.size());
      // TODO: 360 in a 36x10 grid is nice... but is weird with weeks
      //   Is there a width i can make work that is divisible by 7? (Or tall and render vertically?)
      //   i guess 35x10 could be ok... can reduce overall width of the module by a bit if it looks weird
      //   matters less with truly random data... could also consider an "alignment" toggle or something
      for (int i = 0; i < (int)module->values.size(); i++) {
      // for (int i = 0; i < 360; i++) {
      // for (int i = 0; i < 350; i++) {
        int x = 4 + (i % 35) * 5;
        int y = 4 + (i / 35) * 5;

        // NVGcolor color = nvgRGBA(0, 255, 0, rand() * 200 + 55);

        // TODO: This color formula is not great:
        //   Github actually uses multiple, quantized colors
        //   I do need a gradient, but maybe do so with clear cutoffs like them?
        //   Or, at least pick the right one from their set that works well with alpha
        //   Also, feels off w/scaling? non-zero but small values are not visible
        int value = module->values[i];
        NVGcolor color = nvgRGBA(25, 108, 46, value / 10.f * 255.f);

        nvgBeginPath(args.vg);
        nvgRect(args.vg, mm2px(x), mm2px(y), mm2px(4), mm2px(4));
        nvgFillColor(args.vg, color);
        nvgFill(args.vg);

        if (i == 42) {
          nvgStrokeWidth(args.vg, mm2px(0.5));
          nvgStrokeColor(args.vg, nvgRGB(255, 255, 255));
          nvgStroke(args.vg);
        }
      }
    } catch (...) {
      DEBUG("error in Grid::draw");
    }
  }
};

struct EntropyPoolWidget : app::ModuleWidget {
  EntropyPoolWidget(EntropyPool* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/EntropyPool.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


    Grid* grid = createWidget<Grid>(mm2px(Vec(10, 30)));
    grid->module = module;
    grid->setSize(mm2px(Vec(183, 53)));
    addChild(grid);

    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(12, 91)), module, EntropyPool::CLOCK_LIGHT));

    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(23, 91)), module, EntropyPool::RUN_PARAM, EntropyPool::RUN_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(34, 91)), module, EntropyPool::RESET_PARAM, EntropyPool::RESET_LIGHT));
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(45, 91)), module, EntropyPool::RANDOM_PARAM, EntropyPool::RANDOM_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(56, 91)), module, EntropyPool::START_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(67, 91)), module, EntropyPool::LENGTH_PARAM));
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(78, 91)), module, EntropyPool::PING_PONG_PARAM, EntropyPool::PING_PONG_LIGHT));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(89, 91)), module, EntropyPool::FILTER_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(100, 91)), module, EntropyPool::SCALE_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(12, 113)), module, EntropyPool::CLOCK_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(23, 113)), module, EntropyPool::RUN_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(34, 113)), module, EntropyPool::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(45, 113)), module, EntropyPool::RANDOM_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(56, 113)), module, EntropyPool::START_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(67, 113)), module, EntropyPool::LENGTH_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(78, 113)), module, EntropyPool::PING_PONG_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(89, 113)), module, EntropyPool::FILTER_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(100, 113)), module, EntropyPool::SCALE_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(166, 113)), module, EntropyPool::CV_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(177, 113)), module, EntropyPool::TRIGGER_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(188, 113)), module, EntropyPool::END_OF_SEQUENCE_OUTPUT));
  }

  void appendContextMenu(ui::Menu* menu) override {
    ModuleWidget::appendContextMenu(menu);

    EntropyPool* m = getModule<EntropyPool>();
    if (!m) return;

    menu->addChild(new ui::MenuSeparator());
    menu->addChild(createMenuItem("Integrations...", "", [=]() {
      IntegrationsModal* modal = new IntegrationsModal([m](const std::vector<float>& values) {
        m->values = values;
      });
      APP->scene->addChild(modal);
    }));
  }

};

Model* modelEntropyPool = createModel<EntropyPool, EntropyPoolWidget>("EntropyPool");
