#pragma once

#include "EntropyBase.hpp"

#include <rack.hpp>

#include <random>

struct Grid : rack::OpaqueWidget {
  Grid();

  void draw(const DrawArgs &args) override;
  void onEnter(const EnterEvent& event) override;
  void onHover(const HoverEvent& event) override;
  void onDragMove(const DragMoveEvent& event) override;
  void onDragEnd(const DragEndEvent& event) override;
  void onLeave(const LeaveEvent& event) override;
  void onButton(const ButtonEvent& event) override;
  void onDoubleClick(const DoubleClickEvent& event) override;

  rack::ui::Tooltip* tooltip;
  int length, rowLength, itemWidth;

  // Will be null in previews - use default rng to generate a preview grid
  EntropyBase* module = nullptr;

  std::mt19937 defaultRng{42u};
  std::uniform_real_distribution<float> defaultDistribution{0.f, 1.f};

private:
  void updateTooltip();

  bool dragging = false;
  float dragDelta = 0.f;
  int hoverIndex = 0;
};

