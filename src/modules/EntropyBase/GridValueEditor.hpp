#pragma once

#include "EntropyBase.hpp"

#include <rack.hpp>

struct GridValueEditor : rack::ui::Menu {
  GridValueEditor(EntropyBase* module, int index);

private:
  rack::ui::TextField* input;
};

