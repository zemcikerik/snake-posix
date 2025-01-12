#define TB_IMPL
#include <termbox2.h>
#include "connect_menu.h"
#include "host_menu.h"
#include "input.h"
#include "main_menu.h"
#include "server_launcher.h"
#include "shm_client.h"
#include "socket_client.h"
#include "terminal.h"
#include "../libsnake/rng.h"

int main() {
    tb_init();
    rng_init();
    server_launcher_init_sig();
    input_init_sig();

    while (1) {
        const main_menu_result_t result = main_menu();

        if (result == MAIN_MENU_EXIT) {
            break;
        }
        if (result == MAIN_MENU_PRIVATE || result == MAIN_MENU_HOST) {
            game_settings_t* settings = host_menu(
                result == MAIN_MENU_PRIVATE ? HOST_MENU_PRIVATE : HOST_MENU_PUBLIC
            );

            if (settings == NULL) {
                continue;
            }

            if (server_launcher_launch_server_process(settings)) {
                if (settings->room_name_ != NULL) {
                    shm_client_run(settings->room_name_);
                } else {
                    socket_client_run("localhost", settings->port_);
                }
            } else {
                terminal_show_error("Failed to launch server process!");
            }

            host_menu_free_result(settings);
            continue;
        }
        if (result == MAIN_MENU_JOIN_LOCAL) {
            char room_name[256];

            if (connect_menu_local(room_name)) {
                shm_client_run(room_name);
            }
        }
        if (result == MAIN_MENU_JOIN_REMOTE) {
            char room_name[256];
            unsigned short port;

            if (connect_menu_remote(room_name, &port)) {
                socket_client_run(room_name, port);
            }
        }
    }

    tb_shutdown();
    return 0;
}
