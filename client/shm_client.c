#include "shm_client.h"

#include <termbox2.h>

#include "input.h"
#include "terminal.h"
#include "renderer/renderer.h"
#include "../libsnake/shm_game_state.h"
#include "../libsnake/ticker.h"

typedef struct shm_client_input_context_t {
    player_manager_t* player_manager_;
    player_id_t player_id_;
    _Atomic player_status_t* player_status_;
    _Atomic direction_t* last_player_direction_;
    atomic_bool* exit_;

    syn_map_t* map_;
    renderer_t* renderer_;
} shm_client_input_context_t;

void shm_client_update_renderer(const change_t* change, void* ctx) {
    if (change->type_ != CHANGE_TILE_STATE) {
        return;
    }

    const change_tile_state_data_t* data = &change->data_.change_tile_state_data_;
    renderer_update(ctx, data->coordinates_.row_, data->coordinates_.column_, data->tile_data_);
}

void shm_client_on_arrow_press(const uint16_t arrow, void* arg) {
    shm_client_input_context_t* ctx = arg;

    if (arrow == TB_KEY_ARROW_UP && *ctx->last_player_direction_ != DIRECTION_DOWN) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_UP);
    } else if (arrow == TB_KEY_ARROW_LEFT && *ctx->last_player_direction_ != DIRECTION_RIGHT) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_LEFT);
    } else if (arrow == TB_KEY_ARROW_DOWN && *ctx->last_player_direction_ != DIRECTION_UP) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_DOWN);
    } else if (arrow == TB_KEY_ARROW_RIGHT && *ctx->last_player_direction_ != DIRECTION_LEFT) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_RIGHT);
    }
}

void shm_client_on_exit_press(void* arg) {
    *((shm_client_input_context_t*) arg)->exit_ = true;
}

void shm_client_on_pause_press(void* arg) {
    shm_client_input_context_t* ctx = arg;

    if (*ctx->player_status_ == PLAYER_PLAYING) {
        player_manager_pause(ctx->player_manager_, ctx->player_id_);
    }
}

void shm_client_on_unpause_press(void* arg) {
    shm_client_input_context_t* ctx = arg;

    if (*ctx->player_status_ == PLAYER_PAUSED) {
        player_manager_unpause(ctx->player_manager_, ctx->player_id_);
    }
}

void shm_client_on_respawn_press(void* arg) {
    shm_client_input_context_t* ctx = arg;

    if (*ctx->player_status_ == PLAYER_DEAD) {
        player_manager_respawn(ctx->player_manager_, ctx->player_id_);
    }
}

void shm_client_on_resize(void* arg) {
    shm_client_input_context_t* ctx = arg;
    renderer_redraw(ctx->renderer_, ctx->map_, *ctx->player_status_);
}

void shm_client_run(const char* room_name) {
    shm_game_state_t game_state;
    if (!shm_game_state_open(&game_state, room_name)) {
        terminal_show_error("Failed to open shared memory with server!");
        return;
    }

    game_state_t* state = shm_game_state_get(&game_state);
    player_id_t player_id;

    if (!player_manager_register(&state->player_manager_, &player_id)) {
        shm_game_state_close(&game_state);
        terminal_show_error("Game room is full!");
        return;
    }

    atomic_bool exit = false;

    ticker_observer_t observer;
    ticker_observer_init(&observer, &state->ticker_);

    player_t init_player_state = player_manager_get_player_state(&state->player_manager_, player_id);
    _Atomic player_status_t last_player_status = init_player_state.status_;
    _Atomic direction_t last_player_direction = init_player_state.direction_;

    renderer_t renderer;
    renderer_init(&renderer);
    renderer_redraw(&renderer, &state->map_, last_player_status);

    shm_client_input_context_t input_context = {
        .player_manager_ = &state->player_manager_,
        .player_id_ = player_id,
        .player_status_ = &last_player_status,
        .last_player_direction_ = &last_player_direction,
        .exit_ = &exit,

        .map_ = &state->map_,
        .renderer_ = &renderer,
    };
    input_callbacks_t input_callbacks = {
        .on_arrow_press = shm_client_on_arrow_press,
        .on_exit_press = shm_client_on_exit_press,
        .on_pause_press = shm_client_on_pause_press,
        .on_unpause_press = shm_client_on_unpause_press,
        .on_respawn_press = shm_client_on_respawn_press,
        .on_resize = shm_client_on_resize,
        .ctx_ = &input_context
    };
    input_t input;
    input_init(&input, &input_callbacks);

    while (!exit && !state->game_ended_) {
        const size_t ticks_passed = ticker_observer_wait_for_next_tick(&observer);

        if (exit || state->game_ended_) {
            break;
        }

        const player_t player_state = player_manager_get_player_state(&state->player_manager_, player_id);
        last_player_direction = player_state.direction_;

        if (ticks_passed > 1 || player_state.status_ != last_player_status) {
            last_player_status = player_state.status_;
            renderer_redraw(&renderer, &state->map_, last_player_status);
        } else {
            syn_changelog_for_each_copy(&state->changelog_, shm_client_update_renderer, &renderer);
        }
    }

    input_destroy(&input);
    renderer_destroy(&renderer);

    if (exit) {
        player_manager_unregister(&state->player_manager_, player_id);
    }

    shm_game_state_close(&game_state);
    terminal_show_info(exit ? "Game exited." : "Game ended.");

}
