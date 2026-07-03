#include "ValuesModal.hpp"
#include "TextFieldContainer.hpp"

#include <sstream>
#include <string>
#include <vector>

using namespace rack;

namespace {
  std::string join(std::vector<float> values) {
    std::ostringstream stream;

    // std::fixed prevents 'e' notation
    stream << std::fixed;

    for (size_t i = 0; i < values.size(); ++i) {
      stream << values[i];
      if (i < values.size() - 1) {
        stream << ",";
      }
    }

    return stream.str();
  }
}

ValuesModal::ValuesModal(EntropyBase* module)
  : Modal(280, 113),
    module(module)
{
  auto label = new ui::MenuLabel();
  label->box.pos = Vec(7, 7);
  label->text = "Values";
  addChild(label);

  valuesField = new ui::TextField();
  valuesField->box.pos = Vec(14, 28);
  valuesField->box.size = Vec(253, 38);
  valuesField->text = join(module->values);
  addChild(TextFieldContainer::wrap(valuesField));

  statusLabel = new ui::Label();
  statusLabel->color = nvgRGB(255, 0, 0);
  statusLabel->box.pos = Vec(7, 84);
  addChild(statusLabel);
}

void ValuesModal::onOpen() {
  APP->event->setSelectedWidget(valuesField);
  valuesField->selectAll();
}

bool ValuesModal::onSave() {
  try {
    auto tokens = string::split(valuesField->text, ",");

    std::vector<float> values;
    for (int i = 0; i < module->totalLength; i++) {
      float value;

      if (i < (int)tokens.size()) {
        std::string token = tokens[i];
        value = string::trim(token).empty() ? 0 : math::clamp(std::stof(token), -1.f, 1.f);
      } else {
        value = 0;
      }

      values.push_back(value);
    }

    module->values = values;
    return true;
  } catch(...) {
    statusLabel->text = "Error";
    return false;
  }
}
