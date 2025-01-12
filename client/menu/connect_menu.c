#include "connect_menu.h"
#include <stdlib.h>
#include <termbox2.h>
#include "../terminal.h"
#include "../../libsnake/coordinates.h"

bool connect_menu_local(char* out_room_name) {
    tb_clear();
    tb_print(0, 0, 0, 0, "Enter room name to join: ");
    tb_present();

    const coordinates_t coordinates = { 0, 25 };
    return terminal_read_string(coordinates, out_room_name) != READ_CANCELLED && strlen(out_room_name) > 0;
}

bool connect_menu_remote(char* out_hostname, unsigned short* out_port) {
    tb_clear();
    tb_print(0, 0, 0, 0, "Enter hostname: ");
    tb_present();

    coordinates_t coordinates = { 0, 16 };
    if (terminal_read_hostname(coordinates, out_hostname) == READ_CANCELLED || strlen(out_hostname) == 0) {
        return false;
    }

    coordinates.row_ = 1;
    coordinates.column_ = 12;
    terminal_read_result_t result;

    do {
        tb_clear();
        tb_printf(0, 0, 0, 0, "Hostname: %s", out_hostname);
        tb_print(0, 1, 0, 0, "Enter port: ");
        tb_present();

        result = terminal_read_port(coordinates, out_port);

        if (result == READ_INVALID) {
            terminal_show_error("Invalid port.");
        }
    } while (result == READ_INVALID);

    return result != READ_CANCELLED && result != READ_EMPTY;
}
