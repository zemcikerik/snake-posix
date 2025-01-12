#include "host_menu.h"
#include <stdlib.h>
#include <termbox2.h>
#include "../terminal.h"
#include "../../libsnake/constants.h"
#include "../../libsnake/rng.h"

coordinate_t host_menu_clamp_coordinate(const coordinate_t min, const coordinate_t val, const coordinate_t max) {
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

game_settings_t* host_menu(const host_menu_options_t options) {
    char buffer[256];
    coordinates_t coordinates;

#define SET_READ_COORDS(x, y)   \
    coordinates.row_ = x;   \
    coordinates.column_ = y;   \

    if (options == HOST_MENU_PUBLIC) {
        tb_clear();
        tb_print(0, 0, 0, 0, "Enter room name (blank to disable local access): ");
        tb_present();

        SET_READ_COORDS(0, 49);
        if (terminal_read_string(coordinates, buffer) == READ_CANCELLED) {
            return NULL;
        }
    } else {
        const int ids[2] = { rng_uniform_dist(0, RAND_MAX), rng_uniform_dist(0, RAND_MAX) };
        sprintf(buffer, "$--private_%d_%d", ids[0], ids[1]);
    }

    char* room_name = NULL;
    unsigned short port = SETTING_NO_PORT;

    if (strlen(buffer) > 0) {
        room_name = malloc(strlen(buffer) + 1);
        strcpy(room_name, buffer);
    }

#define PRINT_ROOM_NAME() if (room_name != NULL) tb_printf(0, 0, 0, 0, "Room name: %s", room_name)
#define ROOM_NAME_OFFSET (room_name != NULL ? 1 : 0)

    if (options == HOST_MENU_PUBLIC) {
        terminal_read_result_t result;

        do {
            tb_clear();
            PRINT_ROOM_NAME();
            tb_print(0, ROOM_NAME_OFFSET, 0, 0, "Enter port (blank to disable remote access): ");
            tb_present();

            SET_READ_COORDS(ROOM_NAME_OFFSET, 45);
            result = terminal_read_port(coordinates, &port);

            if (result == READ_INVALID) {
                terminal_show_error("Invalid port.");
            }
        } while (result == READ_INVALID);

        if (result == READ_CANCELLED) {
            if (room_name != NULL) {
                free(room_name);
            }
            return NULL;
        }
        if (result == READ_EMPTY) {
            port = SETTING_NO_PORT;
        }
    }

#define PRINT_PORT() if (port != SETTING_NO_PORT) tb_printf(0, ROOM_NAME_OFFSET, 0, 0, "Port: %hu", port)
#define PORT_OFFSET (port != SETTING_NO_PORT ? 1 : 0)

    if (room_name == NULL && port == SETTING_NO_PORT) {
        terminal_show_error("At least one access method has to be defined!");
        return NULL;
    }

    tb_clear();
    PRINT_ROOM_NAME();
    PRINT_PORT();
    tb_print(0, ROOM_NAME_OFFSET + PORT_OFFSET, 0, 0, "Enter path to map (empty for blank map): ");
    tb_present();

    SET_READ_COORDS(ROOM_NAME_OFFSET + PORT_OFFSET, 41);
    if (terminal_read_string(coordinates, buffer) == READ_CANCELLED) {
        if (room_name != NULL) {
            free(room_name);
        }
        return NULL;
    }

    char* map_path = NULL;
    coordinate_t width = 0;
    coordinate_t height = 0;

    if (strlen(buffer) > 0) {
        map_path = malloc(strlen(buffer) + 1);
        strcpy(map_path, buffer);
    } else {
        sprintf(buffer, "Map width (min = %d, max = %d): ", MIN_MAP_WIDTH, MAX_MAP_WIDTH);

        tb_clear();
        PRINT_ROOM_NAME();
        PRINT_PORT();
        tb_print(0, ROOM_NAME_OFFSET + PORT_OFFSET, 0, 0, buffer);
        tb_present();

        SET_READ_COORDS(ROOM_NAME_OFFSET + PORT_OFFSET, strlen(buffer));
        if (terminal_read_coordinate(coordinates, &width) == READ_CANCELLED) {
            if (room_name != NULL) {
                free(room_name);
            }
            return NULL;
        }
        width = host_menu_clamp_coordinate(MIN_MAP_WIDTH, width, MAX_MAP_WIDTH);

        sprintf(buffer, "Map height (min = %d, max = %d): ", MIN_MAP_HEIGHT, MAX_MAP_HEIGHT);

        tb_clear();
        PRINT_ROOM_NAME();
        PRINT_PORT();
        tb_printf(0, ROOM_NAME_OFFSET + PORT_OFFSET, 0, 0, "Width: %d", width);
        tb_print(0, 1 + ROOM_NAME_OFFSET + PORT_OFFSET, 0, 0, buffer);
        tb_present();

        SET_READ_COORDS(1 + ROOM_NAME_OFFSET + PORT_OFFSET, strlen(buffer));
        if (terminal_read_coordinate(coordinates, &height) == READ_CANCELLED) {
            if (room_name != NULL) {
                free(room_name);
            }
            return NULL;
        }
        height = host_menu_clamp_coordinate(MIN_MAP_HEIGHT, height, MAX_MAP_HEIGHT);
    }

    game_settings_t* settings = malloc(sizeof(game_settings_t));
    settings->room_name_ = room_name;
    settings->map_path_ = map_path;
    settings->width_ = width;
    settings->height_ = height;
    settings->port_ = port;
    return settings;
}

void host_menu_free_result(game_settings_t* result) {
    free(result->room_name_);

    if (result->map_path_ != NULL) {
        free(result->map_path_);
    }

    free(result);
}
