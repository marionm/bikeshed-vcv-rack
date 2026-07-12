#include "Grid.hpp"
#include "GridValueEditor.hpp"
#include "components/Palette.hpp"
#include "widgets/Popup.hpp"

#include "nanovg.h"
#include <settings.hpp>

using namespace bikeshed;
using namespace rack;

namespace {
  const float rectRadius = 3.f;
  const float gutterWidth = 1;
  const float borderWidth = gutterWidth / 2;
  const float capWidth = gutterWidth / 2;
  const float capRadius = 3.f + mm2px(.5f); 
}

Grid::Grid()
  : tooltip(new ui::Tooltip())
{
  tooltip->visible = false;
  APP->scene->addChild(tooltip);
}

void Grid::configure(int length, int rowLength, float itemWidth) {
  this->length = length;
  this->rowLength = rowLength;
  this->itemWidth = itemWidth;

  float width = (itemWidth + gutterWidth) * rowLength + 1;
  float height = (itemWidth + gutterWidth) * length / rowLength + 1;
  Vec size = mm2px(Vec(width, height));
  setSize(size);
}

void Grid::draw(const DrawArgs& args) {
  if (!module) {
    // Draw content on base layer when module is null, i.e. in previews where drawLayer is not called
    drawContent(args);
  }
}

// Called for layer 1 only, which does not dim with room lights
void Grid::drawLayer(const DrawArgs& args, int layer) {
  if (module) {
    drawContent(args);
  }
}

// Called for layer 1 only, which does not dim with room lights
void Grid::drawContent(const DrawArgs& args) {
  for (int i = 0; i < length; i++) {
    bool isInRange = module ? module -> isInRange(i) : i < 8;
    bool isMuted = module && module->isMuted(i);

    float activeA = 255.f * (settings::rackBrightness * .4f + .6f);
    float inactiveA = 255.f * (settings::rackBrightness * .8f + .2f);
    float r, g, b, a;
    if (isInRange && !isMuted) {
      r = 46.f;
      g = 160.f;
      b = 67.f;
      a = activeA;
    } else {
      r = 96.f;
      g = 96.f;
      b = 96.f;
      a = inactiveA;
    }

    float value = module ? module->values[i] : defaultDistribution(defaultRng);
    const NVGcolor color = nvgRGBA(r, g, b, a * value);

    const int x = gutterWidth + (i % rowLength) * (itemWidth + gutterWidth);
    const int y = gutterWidth + (i / rowLength) * (itemWidth + gutterWidth);

    // Value box
    bool isFiltered = module ? module->maxValue < value || value < module->minValue : false;
    if (isFiltered) {
      nvgBeginPath(args.vg);
      nvgRoundedRect(args.vg, mm2px(x + .2f), mm2px(y + .2f), mm2px(itemWidth - .4f), mm2px(itemWidth - .4f), rectRadius);
      nvgStrokeColor(args.vg, color);
      nvgStrokeWidth(args.vg, mm2px(.4f));
      nvgStroke(args.vg);
    } else {
      nvgBeginPath(args.vg);
      nvgRoundedRect(args.vg, mm2px(x), mm2px(y), mm2px(itemWidth), mm2px(itemWidth), rectRadius);
      nvgFillColor(args.vg, color);
      nvgFill(args.vg);
    }

    if (!isInRange) {
      continue;
    }

    const float top = mm2px(y - borderWidth);
    const float right = mm2px(x + itemWidth + borderWidth);
    const float bottom = mm2px(y + itemWidth + borderWidth);
    const float left = mm2px(x - borderWidth);
    const float cx = mm2px(x + itemWidth / 2.f);
    const float cy = mm2px(y + itemWidth / 2.f);

    // Range blob
    // Maybe nice as a toggle, as it looks cool in normal cases, but is noisy and just kind of weird
    // to deal with edge cases - left/right borders at edges of grid look odd... but losing the caps
    // sucks in some cases? Also, should round the corners - but in the appropriate directions!
    // nvgStrokeColor(args.vg, borderColor);
    // nvgStrokeWidth(args.vg, mm2px(borderWidth));
    // nvgLineCap(args.vg, NVG_ROUND);
    // int iAbove = i - rowLength;
    // if (iAbove < 0) {
    //   iAbove += length;
    // }
    // int iBelow = i + rowLength;
    // if (iBelow >= length) {
    //   iBelow -= length;
    // }
    // if (module ? !module->isInRange(iAbove) && i >= rowLength) {
    //   nvgBeginPath(args.vg);
    //   nvgMoveTo(args.vg, left, top);
    //   nvgLineTo(args.vg, right, top);
    //   nvgStroke(args.vg);
    // }
    // if (module && !module->isInRange(iBelow) && i < length - rowLength) {
    //   nvgBeginPath(args.vg);
    //   nvgMoveTo(args.vg, left, bottom);
    //   nvgLineTo(args.vg, right, bottom);
    //   nvgStroke(args.vg);
    // }
    // if (i == (module ? module->minIndex : 0)) {
    //   nvgBeginPath(args.vg);
    //   nvgMoveTo(args.vg, left, top);
    //   nvgLineTo(args.vg, left, bottom);
    //   nvgStroke(args.vg);
    // }
    // if (i == (module ? module->maxIndex : 7)) {
    //   nvgBeginPath(args.vg);
    //   nvgMoveTo(args.vg, right, top);
    //   nvgLineTo(args.vg, right, bottom);
    //   nvgStroke(args.vg);
    // }

    // Range caps
    nvgStrokeColor(args.vg, nvgRGBA(whiteR, whiteG, whiteB, activeA));
    nvgStrokeWidth(args.vg, mm2px(capWidth));
    nvgLineCap(args.vg, NVG_ROUND);
    if (i == (module ? module->minIndex : 0)) {
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, cx, top);
      nvgArcTo(args.vg, left, top, left, bottom, capRadius);
      nvgArcTo(args.vg, left, bottom, cx, bottom, capRadius);
      nvgLineTo(args.vg, cx, bottom);
      nvgStroke(args.vg);
    }
    if (i == (module ? module->maxIndex : 7)) {
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, cx, top);
      nvgArcTo(args.vg, right, top, right, bottom, capRadius);
      nvgArcTo(args.vg, right, bottom, cx, bottom, capRadius);
      nvgLineTo(args.vg, cx, bottom);
      nvgStroke(args.vg);
    }

    if (i == (module ? module->index : 0)) {
      // Active glow
      nvgBeginPath(args.vg);
      nvgCircle(args.vg, cx, cy, itemWidth * 4.f);
      auto innerColor = nvgRGBA(whiteR, whiteG, whiteB, activeA * .5f * settings::haloBrightness);
      auto outerColor = nvgRGBA(whiteR, whiteG, whiteB, 0.f);
      auto paint = nvgRadialGradient(args.vg, cx, cy, itemWidth, itemWidth * 4.f, innerColor, outerColor);
      nvgFillPaint(args.vg, paint);
      nvgFill(args.vg);

      // Active dot
      nvgBeginPath(args.vg);
      nvgCircle(args.vg, cx, cy, itemWidth / 1.5f);
      nvgFillColor(args.vg, nvgRGBA(whiteR, whiteG, whiteB, activeA));
      nvgFill(args.vg);
    }
  }
}

void Grid::onEnter(const EnterEvent& event) {
  OpaqueWidget::onEnter(event);

  tooltip->visible = true;
}

void Grid::onHover(const HoverEvent& event) {
  OpaqueWidget::onHover(event);

  int x = event.pos.x / box.getWidth() * rowLength;
  int y = event.pos.y / box.getHeight() * ((float)length / rowLength);
  hoverIndex = y * rowLength + x;

  updateTooltip();
}

void Grid::onDragMove(const DragMoveEvent& event) {
  if (settings::allowCursorLock) {
    APP->window->cursorLock();
  }

  int mods = APP->window->getMods();
  float sensitivity;
  if ((mods & GLFW_MOD_CONTROL) && (mods & GLFW_MOD_SHIFT)) {
    sensitivity = 0.00005f;
  } else if (mods & GLFW_MOD_CONTROL) {
    sensitivity = 0.0005f;
  } else {
    sensitivity = 0.005f;
  }

  float delta = event.mouseDelta.y * sensitivity;
  dragDelta += delta;
  module->values[hoverIndex] = math::clamp(module->values[hoverIndex] - delta, 0.f, 1.f);
  updateTooltip();
}

void Grid::onDragEnd(const DragEndEvent& event) {
  if (dragging && dragDelta == 0.f) {
    // Click/drag/dbl-click is weird in rack - handle a "left click" here
    module->toggleMute(hoverIndex);
  }

  dragging = false;

  if (settings::allowCursorLock) {
    APP->window->cursorUnlock();
  }

  OpaqueWidget::onDragEnd(event);
}

void Grid::onLeave(const LeaveEvent& event) {
  OpaqueWidget::onLeave(event);

  tooltip->visible = false;
}

void Grid::updateTooltip() {
  if (hoverIndex >= 0 && (size_t)hoverIndex < module->values.size()) {
    int mods = APP->window->getMods();
    int sigfigs = ((mods & GLFW_MOD_CONTROL) && (mods & GLFW_MOD_SHIFT)) ? 3 : 2;
    std::string label = string::f("Index %i: %." + std::to_string(sigfigs) + "g", hoverIndex, module->values[hoverIndex]);
    if (module->isMuted(hoverIndex)) {
      label += " (muted)";
    }
    tooltip->text = label;
  } else {
    tooltip->text = "";
  }
}

void Grid::onButton(const ButtonEvent& event) {
  if (event.button == GLFW_MOUSE_BUTTON_RIGHT && event.action == GLFW_PRESS) {
    GridValueEditor* editor = new GridValueEditor(module, hoverIndex);
    new Popup(editor, getAbsoluteOffset(event.pos));
    event.consume(this);
    return;
  } else if (event.button == GLFW_MOUSE_BUTTON_LEFT && event.action == GLFW_PRESS) {
    // With onButton overridden, drag events only work if we also consume left clicks
    event.consume(this);
    dragging = true;
    dragDelta = 0.f;
    return;
  } else {
    Widget::onButton(event);
  }

}

void Grid::onDoubleClick(const DoubleClickEvent& event) {
  module->resetValue(hoverIndex);
}
