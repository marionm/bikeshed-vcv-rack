#pragma once

#include <rack.hpp>

struct ModalContainer : rack::widget::OpaqueWidget {
  void draw(const DrawArgs& args) override;
};

struct Modal : rack::ui::MenuOverlay {
  ModalContainer* modal = nullptr;

  Modal(int width, int height);
  void step() override;
};
