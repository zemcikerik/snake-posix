#ifndef INCLUDE_CLIENT_SERVER_LAUNCHER_H
#define INCLUDE_CLIENT_SERVER_LAUNCHER_H

#include <stdbool.h>
#include "../libsnake/game_settings.h"

void server_launcher_init_sig();
bool server_launcher_launch_server_process(game_settings_t* settings);

#endif
