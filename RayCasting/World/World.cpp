#include <print>

#include "World.hpp"

#include "Canvas.hpp"
#include "Map.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Viewport.hpp"

namespace RC::World {

void start() {
    std::println("Starting world");
}

void draw() {
    Viewport::draw();
    Map::draw();
}

void update() {
    Player::update();
    Viewport::update();
    Map::update();
}

} // namespace RC::World
