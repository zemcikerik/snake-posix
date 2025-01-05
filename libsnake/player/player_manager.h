#ifndef INCLUDE_LIBSNAKE_PLAYER_MANAGER_H
#define INCLUDE_LIBSNAKE_PLAYER_MANAGER_H

#include <stdbool.h>
#include "../constants.h"
#include "syn_player.h"

typedef struct player_manager_t {
    syn_player_t players_[MAX_PLAYERS];
} player_manager_t;

typedef void (*player_manager_for_each_fn_t)(player_manager_t* manager, player_id_t player_id, player_t* player, void* ctx);

void player_manager_init(player_manager_t* self);
void player_manager_destroy(player_manager_t* self);

bool player_manager_register(player_manager_t* self, player_id_t* out_player_id);
void player_manager_unregister(player_manager_t* self, player_id_t player_id);
void player_manager_pause(player_manager_t* self, player_id_t player_id);
void player_manager_unpause(player_manager_t* self, player_id_t player_id);
void player_manager_respawn(player_manager_t* self, player_id_t player_id);

player_t player_manager_get_player_state(player_manager_t* self, player_id_t player_id);
player_t player_manager_peek_player_state(player_manager_t* self, player_id_t player_id);
player_status_t player_manager_get_player_status(player_manager_t* self, player_id_t player_id);
void player_manager_set_player_direction(player_manager_t* self, player_id_t player_id, direction_t direction);

void player_manager_for_each(player_manager_t* self, player_manager_for_each_fn_t for_each_fn, void* ctx);

#endif
