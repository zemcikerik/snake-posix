#include "direction.h"

direction_t direction_get_opposite(const direction_t direction) {
    switch (direction) {
        case DIRECTION_UP:
            return DIRECTION_DOWN;
        case DIRECTION_LEFT:
            return DIRECTION_RIGHT;
        case DIRECTION_DOWN:
            return DIRECTION_UP;
        default:
            return DIRECTION_LEFT;
    }
}
