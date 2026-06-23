#pragma once

#include <rack.hpp>

#include <string>

struct Checkbox : rack::widget::OpaqueWidget {
  std::string text;
  bool value = false;

  void draw(const DrawArgs& args) override;
  void onButton(const rack::event::Button& e) override;
};
