#ifndef INCLUDE_CLIENT_RENDERER_COMMAND_H
#define INCLUDE_CLIENT_RENDERER_COMMAND_H

#include "../../libsnake/map/syn_map.h"

typedef enum renderer_command_type_t {
    RENDERER_COMMAND_REDRAW,
    RENDERER_COMMAND_UPDATE,
} renderer_command_type_t;

typedef struct renderer_command_redraw_data_t {
    syn_map_t* map_;
    player_status_t status_;
} renderer_command_redraw_data_t;

typedef struct renderer_command_update_data_t {
    coordinate_t row_;
    coordinate_t column_;
    map_tile_t tile_;
} renderer_command_update_data_t;

typedef union renderer_command_data_t {
    renderer_command_redraw_data_t redraw_;
    renderer_command_update_data_t update_;
} renderer_command_data_t;

typedef struct renderer_command_t {
    renderer_command_type_t type_;
    renderer_command_data_t data_;
} renderer_command_t;

#endif
