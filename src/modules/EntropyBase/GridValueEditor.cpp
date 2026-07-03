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
    float* pValue;
    GridValueEditorInput(float* pValue) : pValue(pValue) {}

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
      if (pValue) {
        *pValue = math::clamp(quantity->getValue(), 0.f, 1.f);
      }

      Popup::close(this);
    }
  };
}

GridValueEditor::GridValueEditor(int index, float* pValue) {
  ui::MenuLabel* label = new ui::MenuLabel();
  label->text = string::f("Index %i value", index);
  addChild(label);

  ui::MenuEntry* inputContainer = new ui::MenuEntry();
  inputContainer->box.size = math::Vec(150.f, 25.f);
  addChild(inputContainer);

  input = new GridValueEditorInput(pValue);
  input->box.size = inputContainer->box.size;
  if (pValue) {
    input->text = string::f("%g", *pValue);
  }
  inputContainer->addChild(input);
}
