#include "GitHubIntegration.hpp"
#include "GitHubModal.hpp"
#include "TextFieldContainer.hpp"

#include <string>

using namespace rack;

GitHubModal::GitHubModal(EntropyBase* module)
  : Modal(387, 154),
    module(module)
{
  text = new ui::MenuLabel();
  text->text =
    "GitHub token\n"
    "  Set to load contribution history as values\n" \
    "  Optionally prefix with <username>@\n" \
    "  Use a classic token with 'repo' and 'read:user' for private data\n" \
    "  The token not saved with the patch";
  text->box.pos = Vec(7, 7);
  addChild(text);

  tokenField = new GitHubTokenField();
  tokenField->box.pos = Vec(14, 79);
  tokenField->box.size = Vec(360, 21);
  addChild(TextFieldContainer::wrap(tokenField));

  weekendsCheckbox = new Checkbox();
  weekendsCheckbox->box.pos = Vec(15, 104);
  weekendsCheckbox->box.size = Vec(150, 21); // TODO: Checkbox should sanely size itself
  weekendsCheckbox->label = "Include weekends";
  addChild(weekendsCheckbox);

  statusLabel = new ui::Label();
  statusLabel->box.pos = Vec(7, 126);
  addChild(statusLabel);
}

bool GitHubModal::onSave() {
  statusLabel->color = nvgRGB(255, 255, 255);
  statusLabel->text = "Loading...";

  GitHubIntegration::fetchNormalizedContributions(
    tokenField->text, module->totalLength, weekendsCheckbox->value,
    [=](GitHubIntegration::Result result) {
      if (result.success) {
        module->setValues(result.values);
        Modal::close(this);
      } else {
        statusLabel->color = nvgRGB(255, 0, 0);
        statusLabel->text = result.error;
      }
    }
  );

  return false;
}
