#ifndef INCLUDE_CLIENT_MAP_MENU_H
#define INCLUDE_CLIENT_MAP_MENU_H

#include <stdatomic.h>
#include "../libsnake/map/syn_map.h"

typedef struct map_menu_t map_menu_t;
typedef void (*map_menu_key_callback_t)(map_menu_t* map_menu, int key, unsigned int character, void* ctx);

struct map_menu_t {
    syn_map_t* map_;
    pthread_mutex_t mutex_;
    pthread_t input_thread_;
    map_menu_key_callback_t callback_;
    void* callback_ctx_;
    player_status_t last_player_status_;
    atomic_bool destroyed_;
};

void map_menu_init(map_menu_t* self, syn_map_t* map, map_menu_key_callback_t callback, void* ctx);
void map_menu_destroy(map_menu_t* self);
void map_menu_update_begin(map_menu_t* self);
void map_menu_update(coordinates_t coordinates, map_tile_t tile);
void map_menu_update_end(map_menu_t* self);
void map_menu_redraw(map_menu_t* self, player_status_t player_status);

#endif
