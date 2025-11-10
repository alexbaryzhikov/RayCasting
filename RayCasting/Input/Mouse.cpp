#include "Mouse.hpp"

#include "Config.h"
#include "Map.hpp"
#include "Player.hpp"

namespace RC::Mouse {

bool buttonLeft = false;
bool buttonRight = false;

void leftButtonPressed() {
    buttonLeft = true;
}

void leftButtonReleased() {
    buttonLeft = false;
}

void rightButtonPressed() {
    buttonRight = true;
}

void rightButtonReleased() {
    buttonRight = false;
}

void moved(float dx, float dy) {
    Player::angle += dx * MOUSE_SENSITIVITY / 1024.0f;
}

void scrolled(float dx, float dy) {
    if (!Map::isVisible()) return;
    if (dy < 0) {
        Map::zoomFactor *= MAP_ZOOM_SPEED;
    } else if (dy > 0) {
        Map::zoomFactor /= MAP_ZOOM_SPEED;
    }
}

} // namespace RC::Mouse
