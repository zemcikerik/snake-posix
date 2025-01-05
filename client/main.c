#define TB_IMPL
#include <termbox2.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "host_menu.h"
#include "main_menu.h"
#include "map_menu.h"
#include "terminal.h"
#include "../libsnake/constants.h"
#include "../libsnake/shm_game_state.h"

atomic_bool launched = false;

void on_server_process_launched(const int) {
    launched = true;
}

bool launch_server_process(host_menu_result_t* host_info) {
    launched = false;
    const pid_t parent = getpid();
    const pid_t pid = fork();

    if (pid == 0) {
        size_t index = 0;
        char* args[9];
        args[index++] = "./server";

        if (host_info->room_name_ != NULL) {
            args[index++] = "-n";
            args[index++] = host_info->room_name_;
        }

        args[index++] = "-w";
        args[index++] = "60";

        args[index++] = "-h";
        args[index++] = "30";

        char parent_pid_buffer[16];
        sprintf(parent_pid_buffer, "%d", parent);
        args[index++] = "-a";
        args[index++] = parent_pid_buffer;

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

int main() {
    signal(SIG_LAUNCHED, on_server_process_launched);
    tb_init();

    while (1) {
        main_menu_result_t result = main_menu();

        if (result == MAIN_MENU_EXIT) {
            break;
        }
        if (result == MAIN_MENU_HOST) {
            host_menu_result_t* host_info = host_menu();

            if (host_info == NULL) {
                continue;
            }

            // ReSharper disable once CppDFAConstantConditions
            if (!launch_server_process(host_info)) {
                coordinates_t coordinates = { 0, 0 };
                terminal_show_error(coordinates, "Failed to launch server process!");
                host_menu_free_result(host_info);
                continue;
            }

            // ReSharper disable once CppDFAUnreachableCode
            shm_game_state_t game_state;
            if (!shm_game_state_open(&game_state, host_info->room_name_)) {
                coordinates_t coordinates = { 0, 0 };
                terminal_show_error(coordinates, "Failed to open shared memory with server!");
                host_menu_free_result(host_info);
                continue;
            }

            game_state_t* state = shm_game_state_get(&game_state);
            host_menu_free_result(host_info);
            player_id_t player_id;

            if (!player_manager_register(&state->player_manager_, &player_id)) {
                coordinates_t coordinates = { 0, 0 };
                terminal_show_error(coordinates, "Game room is full!");
                shm_game_state_close(&game_state);
                continue;
            }

            map_menu_t map_menu;
            map_menu_init(&map_menu, &state->map_);
            map_menu_redraw(&map_menu);

            ticker_observer_t observer;
            ticker_observer_init(&observer, &state->ticker_);

            // todo fix bug with spawning
            while (true) {
                syn_changelog_for_each_copy(&state->changelog_, update_map_menu_from_change, NULL);
                map_menu_update_finished();
                ticker_observer_wait_for_next_tick(&observer);
            }

            shm_game_state_close(&game_state);
        }
    }

    tb_shutdown();

    return 0;
}
