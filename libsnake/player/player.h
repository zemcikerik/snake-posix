#ifndef INCLUDE_LIBSNAKE_PLAYER_H
#define INCLUDE_LIBSNAKE_PLAYER_H

#include "../direction.h"

typedef unsigned short player_id_t;

typedef enum player_status_t {
    PLAYER_NOT_CONNECTED,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_DEAD,
} player_status_t;

typedef struct player_t {
    player_status_t status_;
    direction_t direction_;
} player_t;

#endif
