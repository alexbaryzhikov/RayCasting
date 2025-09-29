//
//  World.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 20.09.25.
//

#include <print>

#include "World.hpp"

#include "Canvas.hpp"
#include "Palette.hpp"
#include "Player.hpp"

namespace RC {

static constexpr uint32_t BG_GRAY = 0xFF202020;

void startWorld() {
    std::println("Starting world");
}

void drawWorld() {
    fillCanvas(BG_GRAY);
    drawPlayer();
}

void updateWorld() {
    updatePlayer();
}

} // namespace RC
