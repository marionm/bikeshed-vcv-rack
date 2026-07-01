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
  void onLeave(const LeaveEvent& event) override;
  void onButton(const ButtonEvent& event) override;

  rack::ui::Tooltip* tooltip;
  int length, rowLength, itemWidth;

  // Will be null in previews - use default rng to generate a preview grid
  EntropyBase* module = nullptr;

  std::mt19937 defaultRng{42u};
  std::uniform_real_distribution<float> defaultDistribution{0.f, 1.f};

private:
  void updateTooltip();

  int hoverIndex = 0;
};

