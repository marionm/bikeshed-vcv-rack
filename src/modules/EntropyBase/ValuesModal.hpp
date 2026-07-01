#pragma once

#include "EntropyBase.hpp"
#include "../../widgets/Modal.hpp"

#include <rack.hpp>

struct ValuesModal : Modal {
  ValuesModal(EntropyBase* module);

  void onOpen() override;
  bool onSave() override;

private:
  EntropyBase* module;
  rack::ui::TextField* valuesField;
  rack::ui::Label* statusLabel;
};
