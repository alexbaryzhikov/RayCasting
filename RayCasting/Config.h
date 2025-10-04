#ifndef Config_h
#define Config_h

// Display
#define CANVAS_WIDTH    400
#define CANVAS_HEIGHT   225
#define CANVAS_SIZE     (CANVAS_WIDTH * CANVAS_HEIGHT)
#define VIEW_SCALE      3
#define VIEW_WIDTH      (CANVAS_WIDTH * VIEW_SCALE)
#define VIEW_HEIGHT     (CANVAS_HEIGHT * VIEW_SCALE)

// Font
#define FONT_MAP        "FontSweet16"
#define FONT_MAP_WIDTH  128
#define FONT_MAP_HEIGHT FONT_MAP_WIDTH
#define FONT_MAP_SIZE   (FONT_MAP_WIDTH * FONT_MAP_WIDTH)
#define GLYPH_WIDTH     (FONT_MAP_WIDTH / 16)
#define GLYPH_HEIGHT    (FONT_MAP_HEIGHT / 8)

// Map
#define MAP_NAME        "map001"
#define MAP_TYPE        "map"

// Player
#define PLAYER_MOVE_SPEED   1.0f
#define PLAYER_TURN_SPEED   0.1f

#endif /* Config_h */
