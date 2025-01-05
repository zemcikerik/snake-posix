#ifndef INCLUDE_CLIENT_HOST_MENU_H
#define INCLUDE_CLIENT_HOST_MENU_H

#include "../libsnake/game_settings.h"

typedef enum host_menu_options_t {
    HOST_MENU_PROMPT_FOR_ROOM_NAME,
    HOST_MENU_PRIVATE_ROOM_NAME,
} host_menu_options_t;

game_settings_t* host_menu(host_menu_options_t options);
void host_menu_free_result(game_settings_t* result);

#endif
