#include "Viewport.hpp"

#include "Canvas.hpp"
#include "Config.h"
#include "Palette.hpp"

namespace RC::Viewport {

constexpr float horizonHeight = CANVAS_HEIGHT / 2.0f;

void draw() {
    Palette::setColor(Palette::BEIGE_DARK);
    Canvas::boxFill(0, 0, CANVAS_WIDTH, horizonHeight);
    Palette::setColor(Palette::BEIGE_LIGHT);
    Canvas::boxFill(0, horizonHeight, CANVAS_WIDTH, CANVAS_HEIGHT - horizonHeight);
}

} // namespace RC::Viewport
