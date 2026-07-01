#pragma once

#include "EntropyBase.hpp"
#include "GitHubTokenField.hpp"
#include "../../widgets/Checkbox.hpp"
#include "../../widgets/Modal.hpp"

#include <rack.hpp>

struct GitHubModal : Modal {
  GitHubModal(EntropyBase* module);

  bool onSave() override;

private:
  EntropyBase* module;
  rack::ui::MenuLabel* text;
  rack::ui::Label* statusLabel;
  GitHubTokenField* tokenField;
  Checkbox* weekendsCheckbox;
};
