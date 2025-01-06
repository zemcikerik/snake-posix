#include "server.h"
#include <stdbool.h>
#include "map_loader.h"

#define PUSH_CHANGELOG_TILE(coord) changelog_push_tile_state_change(data->changelog_, coord, map_get_tile_state(data->map_, coord))

typedef struct process_player_data_t {
    changelog_t* changelog_;
    map_t* map_;
    player_data_cache_t* player_data_cache_;
    size_t fruits_eaten_;
    bool player_died_;
    bool no_player_alive_;
} process_player_data_t;

bool server_init(server_t* self, const game_settings_t* game_settings) {
    if (!shm_game_state_init(&self->shm_game_state_, game_settings->room_name_)) {
        fprintf(stderr, "Failed to create shared game state\n");
        return false;
    }

    map_template_t* template = game_settings->map_path_ != NULL
        ? map_loader_load_template_from_file(game_settings->map_path_)
        : map_loader_create_empty_template(game_settings->width_, game_settings->height_);

    if (template == NULL) {
        fprintf(stderr, "Failed to load map template\n");
        shm_game_state_destroy(&self->shm_game_state_);
        return false;
    }

    game_state_t* game_state = shm_game_state_get(&self->shm_game_state_);
    game_state_init(game_state, template);
    map_loader_free_template(template);
    return true;
}

void server_destroy(server_t* self) {
    game_state_t* game_state = shm_game_state_get(&self->shm_game_state_);
    game_state_destroy(game_state);
    shm_game_state_destroy(&self->shm_game_state_);
}

void move_player_head(process_player_data_t* data, const player_id_t player_id, const coordinates_t next_head) {
    const coordinates_t head = player_data_cache_get_head(data->player_data_cache_, player_id);
    const map_tile_t head_state = map_get_tile_state(data->map_, head);
    map_set_tile_player(data->map_, next_head, player_id, head_state.order_ + 1);
    player_data_cache_set_head(data->player_data_cache_, player_id, next_head);
    PUSH_CHANGELOG_TILE(next_head);
}

void move_player_tail(process_player_data_t* data, const player_id_t player_id) {
    const coordinates_t tail = player_data_cache_get_tail(data->player_data_cache_, player_id);
    const map_tile_t tail_state = map_get_tile_state(data->map_, tail);

    if (tail_state.type_ == TILE_PLAYER && tail_state.player_ == player_id) {
        // was not overwritten in previous iteration of another snake
        const coordinates_t head = player_data_cache_get_head(data->player_data_cache_, player_id);

        if (!coordinates_equal(head, tail)) {
            // is not the same player wrapping around
            map_set_tile_empty(data->map_, tail);
            PUSH_CHANGELOG_TILE(tail);
        }
    }

    coordinates_t next_tail;
    if (!map_find_player_neighbor_with_lowest_order(data->map_, player_id, tail, &next_tail)) {
        fprintf(stderr, "move_player_tail: failed to find next snake tail tile, this should not happen\n");
        exit(EXIT_FAILURE);
    }

    player_data_cache_set_tail(data->player_data_cache_, player_id, next_tail);
    PUSH_CHANGELOG_TILE(next_tail);
}

void kill_player(process_player_data_t* data, const player_id_t player_id, player_t* player) {
    const coordinates_t head = player_data_cache_get_head(data->player_data_cache_, player_id);
    map_mark_player_as_dead(data->map_, player_id, head);
    player->status_ = PLAYER_DEAD;
    data->player_died_ = true;
}

bool map_tile_is_empty_predicate(const map_t* map, const coordinates_t coordinates) {
    return map_is_tile_empty(map, coordinates);
}

void spawn_fruit(process_player_data_t* data) {
    coordinates_t fruit;

    if (map_find_random_matching_predicate(data->map_, map_tile_is_empty_predicate, &fruit)) {
        map_set_tile_fruit(data->map_, fruit);
        PUSH_CHANGELOG_TILE(fruit);
    }
}

bool map_tile_is_empty_with_empty_neighbor_predicate(const map_t* map, const coordinates_t coordinates) {
    return map_is_tile_empty(map, coordinates)
        ? map_find_empty_neighbor(map, coordinates, NULL, NULL)
        : false;
}

bool spawn_player(process_player_data_t* data, const player_id_t player_id, player_t* player) {
    coordinates_t head;
    if (!map_find_random_matching_predicate(data->map_, map_tile_is_empty_with_empty_neighbor_predicate, &head)) {
        return false;
    }

    coordinates_t tail;
    direction_t tail_direction;
    map_find_empty_neighbor(data->map_, head, &tail, &tail_direction);
    player->direction_ = direction_get_opposite(tail_direction);

    map_set_tile_player(data->map_, head, player_id, 1);
    player_data_cache_set_head(data->player_data_cache_, player_id, head);
    PUSH_CHANGELOG_TILE(head);

    map_set_tile_player(data->map_, tail, player_id, 0);
    player_data_cache_set_tail(data->player_data_cache_, player_id, tail);
    PUSH_CHANGELOG_TILE(tail);

    player->status_ = PLAYER_PLAYING;
    return true;
}

bool is_player_paused(player_manager_t* manager, const player_id_t player) {
    return player_manager_peek_player_state(manager, player).status_ == PLAYER_PAUSED;
}

void server_tick_process_player(player_manager_t* manager, const player_id_t player_id, player_t* player, void* ctx) {
    process_player_data_t* data = ctx;

    if (player->status_ == PLAYER_RESPAWNING || player->status_ == PLAYER_JOINING) {
        const bool should_spawn_fruit = player->status_ == PLAYER_JOINING;

        if (spawn_player(data, player_id, player)) {
            if (should_spawn_fruit) {
                spawn_fruit(data);
            }
            data->no_player_alive_ = false;
        } else {
            player->status_ = PLAYER_DEAD;
        }
        return;
    }

    if (player->status_ == PLAYER_DISCONNECTING) {
        kill_player(data, player_id, player);
        player->status_ = PLAYER_NOT_CONNECTED;
        return;
    }

    if (player->status_ == PLAYER_PAUSED) {
        data->no_player_alive_ = false;
        return;
    }

    if (player->status_ != PLAYER_PLAYING) {
        return;
    }

    const coordinates_t head = player_data_cache_get_head(data->player_data_cache_, player_id);
    const coordinates_t next_head = map_move_in_direction(data->map_, head, player->direction_);
    const map_tile_t next_head_state = map_get_tile_state(data->map_, next_head);

    if (next_head_state.type_ == TILE_EMPTY || next_head_state.type_ == TILE_FRUIT) {
        move_player_head(data, player_id, next_head);

        if (next_head_state.type_ == TILE_EMPTY) {
            move_player_tail(data, player_id);
        } else {
            data->fruits_eaten_ += 1;
        }
    } else if (next_head_state.type_ == TILE_PLAYER) {
        const coordinates_t other_tail = player_data_cache_get_tail(data->player_data_cache_, next_head_state.player_);

        if (player_id == next_head_state.player_) {
            if (coordinates_equal(next_head, other_tail)) {
                move_player_head(data, player_id, next_head);
                move_player_tail(data, player_id);
            } else {
                kill_player(data, player_id, player);
                return;
            }
        } else if (!coordinates_equal(next_head, other_tail) || next_head_state.player_ < player_id
            || is_player_paused(manager, next_head_state.player_)) {
            kill_player(data, player_id, player);
            return;
        } else {
            const player_t other_player = player_manager_peek_player_state(manager, next_head_state.player_);
            const coordinates_t other_head = player_data_cache_get_head(data->player_data_cache_, next_head_state.player_);
            const coordinates_t other_next_head = map_move_in_direction(data->map_, other_head, other_player.direction_);
            const map_tile_t other_next_head_state = map_get_tile_state(data->map_, other_next_head);

            if (other_next_head_state.type_ == TILE_EMPTY) {
                move_player_head(data, player_id, next_head);
                move_player_tail(data, player_id);
            } else {
                kill_player(data, player_id, player);
                return;
            }
        }
    } else {
        kill_player(data, player_id, player);
        return;
    }

    data->no_player_alive_ = false;
}

bool server_tick(server_t* self) {
    game_state_t* state = shm_game_state_get(&self->shm_game_state_);
    map_t* map = syn_map_t_acquire(&state->map_);

    changelog_t* changelog = syn_changelog_t_acquire(&state->changelog_);
    changelog_clear(changelog);

    process_player_data_t data = {
        .changelog_ = changelog,
        .map_ = map,
        .player_data_cache_ = &self->player_data_cache_,
        .fruits_eaten_ = 0,
        .player_died_ = false,
        .no_player_alive_ = true,
    };

    player_manager_for_each(&state->player_manager_, server_tick_process_player, &data);

    if (data.player_died_) {
        const coordinate_t width = map_get_width(map);
        const coordinate_t height = map_get_height(map);

        for (coordinate_t i = 0; i < height; ++i) {
            for (coordinate_t j = 0; j < width; ++j) {
                const coordinates_t coordinates = { i, j };

                if (map_is_tile_dead(map, coordinates)) {
                    map_set_tile_empty(map, coordinates);
                    changelog_push_tile_state_change(changelog, coordinates, map_get_tile_state(map, coordinates));
                }
            }
        }
    }

    for (size_t i = 0; i < data.fruits_eaten_; ++i) {
        spawn_fruit(&data);
    }

    syn_changelog_t_release(&state->changelog_);
    syn_map_t_release(&state->map_);
    ticker_tick(&state->ticker_);

    return !data.no_player_alive_;
}

void server_end_game(server_t* self) {
    game_state_t* game_state = shm_game_state_get(&self->shm_game_state_);
    game_state->game_ended_ = true;
}
