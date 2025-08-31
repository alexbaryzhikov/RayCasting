//
//  Painter.cpp
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 05.09.25.
//

#include "Painter.hpp"
#include "Canvas.hpp"
#include "Palette.hpp"

const void* drawFrame() {
    canvas.fill(CYAN);
    return canvas.data();
}
