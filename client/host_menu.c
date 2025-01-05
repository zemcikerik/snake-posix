#include "host_menu.h"
#include <stdlib.h>
#include <termbox2.h>

#include "terminal.h"

host_menu_result_t* host_menu() {
    tb_clear();
    tb_print(0, 0, 0, 0, "Enter room name (empty for no local): ");
    tb_present();

    char buffer[256];
    coordinates_t coordinates = {
        .row_ = 0,
        .column_ = 38,
    };

    if (terminal_read_string(coordinates, buffer) == READ_CANCELLED) {
        return NULL;
    }

    // tb_clear();
    // tb_printf(0, 0, 0, 0, "Room name: %s", buffer);

    char* room_name = malloc(strlen(buffer) + 1);
    strcpy(room_name, buffer);

    host_menu_result_t* result = malloc(sizeof(host_menu_result_t));
    result->room_name_ = room_name;
    return result;
}

void host_menu_free_result(host_menu_result_t* result) {
    free(result->room_name_);
    free(result);
}
