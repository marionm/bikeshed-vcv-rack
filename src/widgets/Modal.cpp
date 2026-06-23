#include "Modal.hpp"

using namespace rack;

void ModalContainer::draw(const DrawArgs& args) {
  nvgBeginPath(args.vg);
  nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 6.f);
  nvgFillColor(args.vg, nvgRGB(40, 40, 40));
  nvgFill(args.vg);

  nvgStrokeWidth(args.vg, 1.f);
  nvgStrokeColor(args.vg, nvgRGB(80, 80, 80));
  nvgStroke(args.vg);

  Widget::draw(args);
}

Modal::Modal(int width, int height) {
  modal = new ModalContainer();
  modal->box.size = Vec(width, height);
  addChild(modal);

  // This main "modal" is actually a full screen box - darken it to act as an backdrop,
  // making the container the actual visual modal
  bgColor = nvgRGBA(0, 0, 0, 160);
}

void Modal::step() {
  if (parent) {
    box.size = parent->box.size;
  }

  // Center modal container
  modal->box.pos = box.size.minus(modal->box.size).div(2.f);

  ui::MenuOverlay::step();
}
