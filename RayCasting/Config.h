#ifndef Config_h
#define Config_h

// Display
#define CANVAS_WIDTH                1280
#define CANVAS_HEIGHT               720
#define CANVAS_SIZE                 (CANVAS_WIDTH * CANVAS_HEIGHT)
#define VIEW_SCALE                  2
#define VIEW_WIDTH                  (CANVAS_WIDTH * VIEW_SCALE)
#define VIEW_HEIGHT                 (CANVAS_HEIGHT * VIEW_SCALE)

// Font
#define FONT_MAP                    "FontSweet16"
#define FONT_MAP_WIDTH              128
#define FONT_MAP_HEIGHT             FONT_MAP_WIDTH
#define FONT_MAP_SIZE               (FONT_MAP_WIDTH * FONT_MAP_WIDTH)
#define GLYPH_WIDTH                 (FONT_MAP_WIDTH / 16)
#define GLYPH_HEIGHT                (FONT_MAP_HEIGHT / 8)

// Map
#define MAP_LOAD                    false
#define MAP_TILE_SIZE               64.0f
#define MAP_SCROLL_SPEED            10.0f
#define MAP_ZOOM_DEFAULT            0.5f
#define MAP_ZOOM_SPEED              1.03124f
#define MAP_WIDTH                   41
#define MAP_HEIGHT                  41
#define MAP_SEED                    80386
#define FLOOR_COVERAGE_TARGET       0.5f
#define ROOM_MIN_SIZE               2
#define ROOM_MAX_SIZE               10
#define ROOM_MAX_LOOPS              10000

// Player
#define PLAYER_MOVE_SPEED           4.0f
#define PLAYER_TURN_SPEED           0.02f
#define PLAYER_ACCELERATION         0.4f
#define PLAYER_FRICTION             0.1f
#define PLAYER_RADIUS               16.0f

// Camera
#define CAMERA_HEIGHT               32.0f
#define CAMERA_FOV                  1.04719758f
#define CAMERA_NEAR_CLIP            12.0f

// Mouse
#define MOUSE_SENSITIVITY           1.0f

#endif /* Config_h */
