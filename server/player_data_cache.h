#ifndef INCLUDE_SERVER_PLAYER_DATA_CACHE
#define INCLUDE_SERVER_PLAYER_DATA_CACHE

#include "../libsnake/constants.h"
#include "../libsnake/coordinates.h"
#include "../libsnake/player/player.h"

typedef struct player_data_cache_t {
    coordinates_t head_[MAX_PLAYERS];
    coordinates_t tail_[MAX_PLAYERS];
} player_data_cache_t;

coordinates_t player_data_cache_get_head(const player_data_cache_t* self, player_id_t player);
coordinates_t player_data_cache_get_tail(const player_data_cache_t* self, player_id_t player);
void player_data_cache_set_head(player_data_cache_t* self, player_id_t player, coordinates_t coordinates);
void player_data_cache_set_tail(player_data_cache_t* self, player_id_t player, coordinates_t coordinates);

#endif
