#include "player_manager.h"
#include <stdlib.h>

void player_manager_init(player_manager_t* self) {
    self->registered_players_count_ = 0;
    init_pthread_shared_mutex(&self->registration_mutex_);

    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        syn_player_t_init(&self->players_[i]);
    }
}

void player_manager_destroy(player_manager_t* self) {
    pthread_mutex_destroy(&self->registration_mutex_);

    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        syn_player_t_destroy(&self->players_[i]);
    }
}

player_id_t player_manager_register(player_manager_t* self) {
    pthread_mutex_lock(&self->registration_mutex_);

    if (self->registered_players_count_ == MAX_PLAYERS) {
        pthread_mutex_unlock(&self->registration_mutex_);
        return PLAYER_REGISTRATION_FULL;
    }

    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        player_t* player = syn_player_t_acquire(&self->players_[i]);

        if (player->status_ != PLAYER_NOT_CONNECTED) {
            syn_player_t_release(&self->players_[i]);
            continue;
        }

        player->status_ = PLAYER_PAUSED;
        player->direction_ = DIRECTION_UP;
        syn_player_t_release(&self->players_[i]);

        self->registered_players_count_ += 1;
        pthread_mutex_unlock(&self->registration_mutex_);
        return i;
    }

    fprintf(stderr, "player_manager_register: failed to find non-registered player.\n");
    exit(EXIT_FAILURE);
}

void player_manager_unregister(player_manager_t* self, const player_id_t player_id) {
    pthread_mutex_lock(&self->registration_mutex_);

    player_t* player = syn_player_t_acquire(&self->players_[player_id]);
    player->status_ = PLAYER_NOT_CONNECTED;
    syn_player_t_release(&self->players_[player_id]);

    self->registered_players_count_ -= 1;
    pthread_mutex_unlock(&self->registration_mutex_);
}

player_t player_manager_get_player_state(player_manager_t* self, const player_id_t player_id) {
    const player_t copy = *syn_player_t_acquire(&self->players_[player_id]);
    syn_player_t_release(&self->players_[player_id]);
    return copy;
}

player_t player_manager_peek_player_state(player_manager_t* self, const player_id_t player_id) {
    return *syn_player_t_peek(&self->players_[player_id]);
}

void player_manager_set_player_direction(player_manager_t* self, const player_id_t player_id, const direction_t direction) {
    player_t* player = syn_player_t_acquire(&self->players_[player_id]);
    player->direction_ = direction;
    syn_player_t_release(&self->players_[player_id]);
}

typedef struct player_entry_t {
    player_id_t player_id_;
    player_t* player_;
} player_entry_t;

void player_manager_for_each(player_manager_t* self, const player_manager_for_each_fn_t for_each_fn, void* ctx) {
    pthread_mutex_lock(&self->registration_mutex_);

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

    pthread_mutex_unlock(&self->registration_mutex_);
}
