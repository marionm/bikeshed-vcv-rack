#pragma once

#include "Bikeshed.hpp"
#include "components/Palette.hpp"

#include <nanovg.h>

#include <string>
#include <math.h>

namespace bikeshed {

namespace {
  constexpr float ARC_GAP = 1.f;
  constexpr float MIN_ANGLE = -0.75f * M_PI;
  constexpr float MAX_ANGLE = -MIN_ANGLE;

  // TODO: Not right - should be a TransparentWidget added as a child to a FramebufferWidget
  //       Overriding FramebufferWidget::draw overrides the caching it does internally
  struct KnobArc : rack::FramebufferWidget {
    KnobArc(float r, bool centerPip, bool minPip)
      : r(r),
        centerPip(centerPip),
        minPip(minPip)
    {}

    void draw(const DrawArgs& args) override {
      float minAngle = MIN_ANGLE - M_PI / 2.f;
      float maxAngle = MAX_ANGLE - M_PI / 2.f;

      nvgStrokeColor(args.vg, white);
      nvgStrokeWidth(args.vg, mm2px(.232f));

      nvgBeginPath(args.vg);
      nvgArc(args.vg, mm2px(r), mm2px(r), mm2px(r + ARC_GAP), minAngle, maxAngle, NVG_CW);
      nvgLineCap(args.vg, NVG_ROUND);
      nvgStroke(args.vg);

      if (centerPip) {
        // Line pip
        // nvgBeginPath(args.vg);
        // nvgMoveTo(args.vg, mm2px(r), mm2px(r));
        // nvgLineTo(args.vg, mm2px(r), -mm2px(ARC_GAP));
        // nvgStroke(args.vg);

        // Dot pip
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, mm2px(r), -mm2px(ARC_GAP), mm2px(ARC_GAP / 2.f));
        nvgFillColor(args.vg, white);
        nvgFill(args.vg);
      }

      if (minPip) {
        // Line pip
        // nvgBeginPath(args.vg);
        // nvgMoveTo(args.vg, mm2px(r), mm2px(r));
        // nvgLineTo(
        //   args.vg,
        //   mm2px(r + (r + ARC_GAP) * std::cos(minAngle)),
        //   mm2px(r + (r + ARC_GAP) * std::sin(minAngle))
        // );
        // nvgStroke(args.vg);

        // Dot pip
        nvgBeginPath(args.vg);
        nvgCircle(
          args.vg,
          mm2px(r + (r + ARC_GAP) * std::cos(minAngle)),
          mm2px(r + (r + ARC_GAP) * std::sin(minAngle)),
          mm2px(ARC_GAP / 2.f)
        );
        nvgFillColor(args.vg, white);
        nvgFill(args.vg);
      }
    }

    float r;
    bool centerPip = false;
    bool minPip = false;
  };

  struct Knob : rack::app::SvgKnob {
    Knob(std::string bgPath, std::string fgPath, float r, bool centerPip, bool minPip) {
      minAngle = MIN_ANGLE;
      maxAngle = MAX_ANGLE;

      bg = new rack::widget::SvgWidget;
      fb->addChildBelow(bg, tw);

      setSvg(rack::Svg::load(rack::asset::plugin(pluginInstance, fgPath)));
      bg->setSvg(rack::Svg::load(rack::asset::plugin(pluginInstance, bgPath)));

      addChildBottom(new KnobArc(r, centerPip, minPip));
    }

    rack::widget::SvgWidget* bg;
  };
}

template <bool centerPip = true, bool minPip = false>
struct SmallKnob : Knob {
  SmallKnob()
    : Knob("res/components/knob_sm_bg.svg", "res/components/knob_sm_fg.svg", 3.f, centerPip, minPip)
  {}
};

template <bool centerPip = true, bool minPip = false>
struct MediumKnob : Knob {
  MediumKnob()
    : Knob("res/components/knob_md_bg.svg", "res/components/knob_md_fg.svg", 5.f, centerPip, minPip)
  {}
};

} // namespace bikeshed
