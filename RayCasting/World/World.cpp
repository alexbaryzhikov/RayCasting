#include "World.hpp"

#include "Canvas.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Text.hpp"

namespace RC::World {

constexpr uint32_t BG_GRAY = 0xFF202020;

void start() {
    std::printf("Starting world");
}

void draw() {
    Canvas::fill(BG_GRAY);
    Player::draw();
}

void update() {
    Player::update();
}

} // namespace RC::World
