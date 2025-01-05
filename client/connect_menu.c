#include "connect_menu.h"
#include <stdlib.h>
#include <termbox2.h>
#include "terminal.h"
#include "../libsnake/coordinates.h"

char* connect_menu() {
    tb_clear();
    tb_print(0, 0, 0, 0, "Enter room name to join: ");
    tb_present();

    char buffer[256];
    const coordinates_t coordinates = { 0, 25 };

    if (terminal_read_string(coordinates, buffer) == READ_CANCELLED || strlen(buffer) == 0) {
        return NULL;
    }

    char* copy = malloc(strlen(buffer) + 1);
    strcpy(copy, buffer);
    return copy;
}

void connect_menu_free_result(char* result) {
    free(result);
}
