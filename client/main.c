#define TB_IMPL
#include <termbox2.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "connect_menu.h"
#include "host_menu.h"
#include "main_menu.h"
#include "map_menu.h"
#include "terminal.h"
#include "../libsnake/constants.h"
#include "../libsnake/rng.h"
#include "../libsnake/shm_game_state.h"

atomic_bool launched = false;

void on_server_process_launched(const int) {
    launched = true;
}

bool launch_server_process(game_settings_t* settings) {
    launched = false;
    settings->parent_ = getpid();
    const pid_t pid = fork();

    if (pid == 0) {
        size_t index = 0;
        char* args[11];

#define DECLARE_INT_ARG_BUFFER(name) char name##_buffer[16]
#define PUSH_INT_ARG(name, flag, arg)   \
        sprintf(name##_buffer, "%d", (int) arg);    \
        args[index++] = flag;   \
        args[index++] = name##_buffer;

        args[index++] = "./server";
        args[index++] = "-n";
        args[index++] = settings->room_name_;

        DECLARE_INT_ARG_BUFFER(width);
        DECLARE_INT_ARG_BUFFER(height);

        if (settings->map_path_) {
            args[index++] = "-m";
            args[index++] = settings->map_path_;
        } else {
            PUSH_INT_ARG(width, "-w", settings->width_);
            PUSH_INT_ARG(height, "-h", settings->height_);
        }

        DECLARE_INT_ARG_BUFFER(parent);
        PUSH_INT_ARG(parent, "-a", settings->parent_);

        args[index] = NULL;
        execv("./server", args);

        perror("Failed to launch server process");
        exit(EXIT_FAILURE);
    }

    sleep(3);

    if (launched) {
        return true;
    }

    kill(pid, SIGKILL);
    return false;
}

void update_map_menu_from_change(const change_t* change, void*) {
    if (change->type_ != CHANGE_TILE_STATE) {
        return;
    }

    const change_tile_state_data_t* data = &change->data_.change_tile_state_data_;
    map_menu_update(data->coordinates_, data->tile_data_);
}

typedef struct key_press_context_t {
    player_manager_t* player_manager_;
    atomic_bool* exit_;
    player_id_t player_id_;
} key_press_context_t;

void handle_key_press(map_menu_t*, const int key, const unsigned int character, void* arg) {
    key_press_context_t* ctx = *(key_press_context_t**) arg;

    if (key == TB_KEY_ARROW_UP) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_UP);
    } else if (key == TB_KEY_ARROW_LEFT) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_LEFT);
    } else if (key == TB_KEY_ARROW_DOWN) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_DOWN);
    } else if (key == TB_KEY_ARROW_RIGHT) {
        player_manager_set_player_direction(ctx->player_manager_, ctx->player_id_, DIRECTION_RIGHT);
    } else if (key == TB_KEY_ESC || key == TB_KEY_CTRL_C) {
        *ctx->exit_ = true;
    } else if (character == 'p') {
        player_manager_pause(ctx->player_manager_, ctx->player_id_);
    } else if (character == 'u') {
        player_manager_unpause(ctx->player_manager_, ctx->player_id_);
    } else if (character == 'r') {
        player_manager_respawn(ctx->player_manager_, ctx->player_id_);
    }
}

void join_local_game(const char* room_name) {
    shm_game_state_t game_state;
    if (!shm_game_state_open(&game_state, room_name)) {
        const coordinates_t coordinates = { 0, 0 };
        terminal_show_error(coordinates, "Failed to open shared memory with server!");
        return;
    }

    game_state_t* state = shm_game_state_get(&game_state);
    player_id_t player_id;

    if (!player_manager_register(&state->player_manager_, &player_id)) {
        shm_game_state_close(&game_state);
        const coordinates_t coordinates = { 0, 0 };
        terminal_show_error(coordinates, "Game room is full!");
        return;
    }

    ticker_observer_t observer;
    ticker_observer_init(&observer, &state->ticker_);

    key_press_context_t* key_press_context_ptr = NULL;
    player_status_t last_player_status = player_manager_get_player_status(&state->player_manager_, player_id);

    map_menu_t map_menu;
    map_menu_init(&map_menu, &state->map_, handle_key_press, &key_press_context_ptr);
    map_menu_redraw(&map_menu, last_player_status);

    atomic_bool exit = false;

    key_press_context_t key_press_context = {
        .player_manager_ = &state->player_manager_,
        .exit_ = &exit,
        .player_id_ = player_id,
    };
    key_press_context_ptr = &key_press_context;

    while (true) {
        if (exit || state->game_ended_) {
            break;
        }

        const size_t ticks_passed = ticker_observer_wait_for_next_tick(&observer);

        if (exit || state->game_ended_) {
            break;
        }

        const player_status_t player_status = player_manager_get_player_status(&state->player_manager_, player_id);

        if (ticks_passed > 1 || player_status != last_player_status) {
            map_menu_redraw(&map_menu, player_status);
            last_player_status = player_status;;
        } else {
            map_menu_update_begin(&map_menu);
            syn_changelog_for_each_copy(&state->changelog_, update_map_menu_from_change, NULL);
            map_menu_update_end(&map_menu);
        }
    }

    if (exit) {
        player_manager_unregister(&state->player_manager_, player_id);
    }

    map_menu_destroy(&map_menu);
    shm_game_state_close(&game_state);
}

int main() {
    signal(SIG_LAUNCHED, on_server_process_launched);
    tb_init();
    init_rng();

    while (1) {
        const main_menu_result_t result = main_menu();

        if (result == MAIN_MENU_EXIT) {
            break;
        }
        if (result == MAIN_MENU_PRIVATE || result == MAIN_MENU_HOST) {
            game_settings_t* settings = host_menu(
                result == MAIN_MENU_PRIVATE
                    ? HOST_MENU_PRIVATE_ROOM_NAME
                    : HOST_MENU_PROMPT_FOR_ROOM_NAME
            );

            if (settings == NULL) {
                continue;
            }

            if (launch_server_process(settings)) {
                join_local_game(settings->room_name_);
            } else {
                const coordinates_t coordinates = { 0, 0 };
                terminal_show_error(coordinates, "Failed to launch server process!");
            }

            host_menu_free_result(settings);
            continue;
        }
        if (result == MAIN_MENU_JOIN) {
            char* room_name = connect_menu();

            if (room_name == NULL) {
                continue;
            }

            join_local_game(room_name);
            connect_menu_free_result(room_name);
        }
    }

    tb_shutdown();

    return 0;
}
