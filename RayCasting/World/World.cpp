#include <chrono>

#include "World.hpp"

#include "Canvas.hpp"
#include "Map.hpp"
#include "Palette.hpp"
#include "Player.hpp"
#include "Text.hpp"
#include "Viewport.hpp"

namespace RC::World {

int64_t fps = 0;

void start() {
    Map::initialize();
    Viewport::initialize();
}

void drawFPS() {
    Palette::color = Palette::white;
    Text::draw(4, 4, std::to_string(fps));
}

void draw() {
    Viewport::draw();
    Map::draw();
    drawFPS();
}

void updateFPS() {
    using namespace std::chrono;
    static time_point lastTime = steady_clock::now();
    time_point currentTime = steady_clock::now();
    if (currentTime != lastTime) {
        microseconds duration = duration_cast<microseconds>(currentTime - lastTime);
        fps = duration_cast<microseconds>(seconds(1)).count() / duration.count();
        lastTime = currentTime;
    }
}

void update() {
    Player::update();
    Viewport::update();
    Map::update();
    updateFPS();
}

} // namespace RC::World
