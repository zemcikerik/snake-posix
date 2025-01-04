#include "player_data_cache.h"

coordinates_t player_data_cache_get_head(const player_data_cache_t* self, const player_id_t player) {
    return self->head_[player];
}

coordinates_t player_data_cache_get_tail(const player_data_cache_t* self, const player_id_t player) {
    return self->tail_[player];
}

void player_data_cache_set_head(player_data_cache_t* self, const player_id_t player, const coordinates_t coordinates) {
    self->head_[player] = coordinates;
}

void player_data_cache_set_tail(player_data_cache_t* self, const player_id_t player, const coordinates_t coordinates) {
    self->tail_[player] = coordinates;
}
