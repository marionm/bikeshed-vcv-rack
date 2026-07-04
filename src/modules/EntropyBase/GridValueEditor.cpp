#include "GridValueEditor.hpp"
#include "../../widgets/Popup.hpp"

using namespace rack;

namespace {
  struct GridValueQuantity : Quantity {
    float getValue() override {
      return value;
    }

    void setValue(float value) override {
      this->value = value;
    }

    float value;
  };

  struct GridValueEditorInput : ui::TextField {
    GridValueEditorInput(EntropyBase* module, int index)
      : module(module),
        index(index)
    {
      text = string::f("%g", module->values[index]);
    }

    bool wasFocused = false;
    void draw(const DrawArgs& args) override{
      ui::TextField::draw(args);

      if (!wasFocused) {
        APP->event->setSelectedWidget(this);
        this->selectAll();
        wasFocused = true;
      }
    }

    void onAction(const ActionEvent& e) override {
      ui::TextField::onAction(e);

      auto quantity = new GridValueQuantity();
      quantity->setDisplayValueString(this->text);
      module->values[index] = math::clamp(quantity->getValue(), 0.f, 1.f);

      Popup::close(this);
    }

    EntropyBase* module;
    int index;
  };
}

GridValueEditor::GridValueEditor(EntropyBase* module, int index) {
  std::string label = string::f("Index %i: %g", index, module->values[index]);
  if (module->isMuted(index)) {
    label += " (muted)";
  }
  addChild(createMenuLabel(label));

  ui::MenuEntry* inputContainer = new ui::MenuEntry();
  inputContainer->box.size = math::Vec(150.f, 25.f);
  addChild(inputContainer);

  input = new GridValueEditorInput(module, index);
  input->box.size = inputContainer->box.size;
  inputContainer->addChild(input);

  addChild(createMenuItem(string::translate("ParamWidget.initialize"), string::translate("key.doubleClick"), [=]() {
    module->resetValue(index);
  }));

  addChild(createMenuItem(module->isMuted(index) ? "Unmute" : "Mute", string::translate("key.click"), [=]() {
    module->toggleMute(index);
  }));

  addChild(createMenuItem(string::translate("ParamWidget.fine"), widget::getKeyCommandName(0, RACK_MOD_CTRL) + string::translate("key.drag"), NULL, true));
}
