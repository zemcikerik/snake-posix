#ifndef INCLUDE_LIBSNAKE_DIRECTION_H
#define INCLUDE_LIBSNAKE_DIRECTION_H

typedef enum direction_t {
    DIRECTION_UP,
    DIRECTION_LEFT,
    DIRECTION_DOWN,
    DIRECTION_RIGHT,
} direction_t;

direction_t direction_get_opposite(direction_t direction);

#endif
