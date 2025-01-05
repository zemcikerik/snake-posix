#ifndef INCLUDE_CLIENT_MAP_MENU_H
#define INCLUDE_CLIENT_MAP_MENU_H

#include "../libsnake/map/syn_map.h"

typedef struct map_menu_t {
    syn_map_t* map_;
} map_menu_t;

void map_menu_init(map_menu_t* self, syn_map_t* map);
void map_menu_update(coordinates_t coordinates, map_tile_t tile);
void map_menu_update_finished();
void map_menu_redraw(map_menu_t* self);

#endif
