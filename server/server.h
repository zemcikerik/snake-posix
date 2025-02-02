#ifndef INCLUDE_SERVER_SERVER_H
#define INCLUDE_SERVER_SERVER_H

#include "client_connection_handler.h"
#include "player_data_cache.h"
#include "../libsnake/game_settings.h"
#include "../libsnake/shm_game_state.h"

typedef struct server_t {
    game_state_t* game_state_;
    shm_game_state_t* shm_game_state_;
    client_connection_handler_t* connection_handler_;
    player_data_cache_t player_data_cache_;
} server_t;

bool server_init(server_t* self, const game_settings_t* game_settings);
void server_destroy(server_t* self);
bool server_tick(server_t* self);
void server_end_game(server_t* self);

#endif
