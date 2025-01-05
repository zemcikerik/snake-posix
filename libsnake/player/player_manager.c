#include "player_manager.h"

void player_manager_init(player_manager_t* self) {
    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        syn_player_t_init(&self->players_[i]);
    }
}

void player_manager_destroy(player_manager_t* self) {
    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        syn_player_t_destroy(&self->players_[i]);
    }
}

bool player_manager_register(player_manager_t* self, player_id_t* out_player_id) {
    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        player_t* player = syn_player_t_acquire(&self->players_[i]);

        if (player->status_ != PLAYER_NOT_CONNECTED) {
            syn_player_t_release(&self->players_[i]);
            continue;
        }

        player->status_ = PLAYER_JOINING;
        syn_player_t_release(&self->players_[i]);
        *out_player_id = i;
        return true;
    }

    return false;
}

void player_manager_unregister(player_manager_t* self, const player_id_t player_id) {
    player_t* player = syn_player_t_acquire(&self->players_[player_id]);

    player->status_ = player->status_ == PLAYER_DEAD || player->status_ == PLAYER_RESPAWNING || player->status_ == PLAYER_JOINING
        ? PLAYER_NOT_CONNECTED
        : PLAYER_DISCONNECTING;

    syn_player_t_release(&self->players_[player_id]);
}

void player_manager_pause(player_manager_t* self, const player_id_t player_id) {
    player_t* player = syn_player_t_acquire(&self->players_[player_id]);

    if (player->status_ == PLAYER_PLAYING) {
        player->status_ = PLAYER_PAUSED;
    }

    syn_player_t_release(&self->players_[player_id]);
}

void player_manager_unpause(player_manager_t* self, const player_id_t player_id) {
    player_t* player = syn_player_t_acquire(&self->players_[player_id]);

    if (player->status_ == PLAYER_PAUSED) {
        player->status_ = PLAYER_PLAYING;
    }

    syn_player_t_release(&self->players_[player_id]);
}

void player_manager_respawn(player_manager_t* self, const player_id_t player_id) {
    player_t* player = syn_player_t_acquire(&self->players_[player_id]);

    if (player->status_ == PLAYER_DEAD) {
        player->status_ = PLAYER_RESPAWNING;
    }

    syn_player_t_release(&self->players_[player_id]);
}

player_t player_manager_get_player_state(player_manager_t* self, const player_id_t player_id) {
    const player_t copy = *syn_player_t_acquire(&self->players_[player_id]);
    syn_player_t_release(&self->players_[player_id]);
    return copy;
}

player_t player_manager_peek_player_state(player_manager_t* self, const player_id_t player_id) {
    return *syn_player_t_peek(&self->players_[player_id]);
}

player_status_t player_manager_get_player_status(player_manager_t* self, const player_id_t player_id) {
    const player_status_t status = syn_player_t_acquire(&self->players_[player_id])->status_;
    syn_player_t_release(&self->players_[player_id]);
    return status;
}

void player_manager_set_player_direction(player_manager_t* self, const player_id_t player_id, const direction_t direction) {
    player_t* player = syn_player_t_acquire(&self->players_[player_id]);
    player->direction_ = direction;
    syn_player_t_release(&self->players_[player_id]);
}

void player_manager_for_each(player_manager_t* self, const player_manager_for_each_fn_t for_each_fn, void* ctx) {
    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        syn_player_t_acquire(&self->players_[i]);
    }

    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        player_t* player = syn_player_t_peek(&self->players_[i]);

        if (player->status_ != PLAYER_NOT_CONNECTED) {
            for_each_fn(self, i, player, ctx);
        }
    }

    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        syn_player_t_release(&self->players_[i]);
    }
}
