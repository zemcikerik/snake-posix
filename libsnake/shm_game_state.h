#ifndef INCLUDE_LIBSNAKE_SHM_GAME_STATE_H
#define INCLUDE_LIBSNAKE_SHM_GAME_STATE_H

#include <stdbool.h>
#include "game_state.h"

typedef struct shm_game_state_t {
    game_state_t* game_state_;
    char** name_;
    int fd_;
} shm_game_state_t;

bool shm_game_state_init(shm_game_state_t* self, const char* room_name);
void shm_game_state_destroy(shm_game_state_t* self);

bool shm_game_state_open(shm_game_state_t* self, const char* room_name);
void shm_game_state_close(shm_game_state_t* self);

game_state_t* shm_game_state_get(shm_game_state_t* self);

#endif
