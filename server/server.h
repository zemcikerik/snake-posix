#ifndef INCLUDE_SERVER_SERVER_H
#define INCLUDE_SERVER_SERVER_H

#include "map_settings.h"
#include "player_data_cache.h"
#include "../libsnake/game_state.h"
#include "../libsnake/shm_game_state.h"

typedef struct server_t {
    union {
        game_state_t* game_state_;
        shm_game_state_t* shm_game_state_;
    };
    player_data_cache_t player_data_cache_;
    bool is_shm_;
} server_t;

void server_init(server_t* self, map_settings_t settings, const char* room_name);
void server_destroy(server_t* self);

bool server_tick(server_t* self);
game_state_t* server_get_game_state(server_t* self);

#endif
