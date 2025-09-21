//
//  WorldPresenter.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 21.09.25.
//

#include "WorldPresenter.hpp"
#include "Canvas.hpp"
#include "World.hpp"

namespace RC {

static std::vector<Segment> playerGeometry = {
    {{0, 0}, {0, 10}},
    {{-5, 0}, {5, 0}},
};

void drawPlayer() {
    const Player& player = World::get().player;
}

void WorldPresenter::drawWorld() {
    drawPlayer();
}

} // namespace RC
