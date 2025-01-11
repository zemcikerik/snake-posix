#ifndef INCLUDE_LIBSNAKE_GAME_SETTINGS_H
#define INCLUDE_LIBSNAKE_GAME_SETTINGS_H

#include <pthread.h>
#include <netinet/in.h>
#include "coordinates.h"

#define SETTING_NO_PARENT 0
#define SETTING_NO_PORT 0

typedef struct game_settings_t {
    char* room_name_;
    char* map_path_;
    coordinate_t width_;
    coordinate_t height_;
    pid_t parent_;
    in_port_t port_;
} game_settings_t;

#endif
