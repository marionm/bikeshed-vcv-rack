#pragma once

#include <rack.hpp>

// Text in rack::ui::TextField that wraps to a second line annoyingly renders two pixels outside of
// the styled container - use this as a wrapper to clip appropriately
struct TextFieldContainer : rack::widget::Widget {
  static TextFieldContainer* wrap(rack::ui::TextField* textField) {
    auto container = new TextFieldContainer();

    container->box.pos = textField->box.pos;
    textField->box.pos = rack::math::Vec(0.f, 0.f);

    container->box.size.x = textField->box.size.x;
    container->box.size.y = textField->box.size.y - 2.f;

    container->addChild(textField);

    return container;
  }
};
