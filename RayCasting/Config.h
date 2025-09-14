//
//  Config.h
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 01.09.25.
//

#ifndef Config_h
#define Config_h

#define CANVAS_WIDTH    800
#define CANVAS_HEIGHT   450
#define CANVAS_SIZE     (CANVAS_WIDTH * CANVAS_HEIGHT)
#define VIEW_SCALE      2
#define VIEW_WIDTH      (CANVAS_WIDTH * VIEW_SCALE)
#define VIEW_HEIGHT     (CANVAS_HEIGHT * VIEW_SCALE)
#define FONT_MAP_WIDTH  128
#define FONT_MAP_HEIGHT FONT_MAP_WIDTH
#define FONT_MAP_SIZE   (FONT_MAP_WIDTH * FONT_MAP_WIDTH)
#define GLYPH_WIDTH     (FONT_MAP_WIDTH / 16)
#define GLYPH_HEIGHT    (FONT_MAP_HEIGHT / 8)

#endif /* Config_h */
