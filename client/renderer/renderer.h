#ifndef INCLUDE_CLIENT_RENDERER_H
#define INCLUDE_CLIENT_RENDERER_H

#include <stdatomic.h>
#include <pthread.h>
#include "renderer_command_queue.h"
#include "../libsnake/map/syn_map.h"

typedef struct renderer_t {
    pthread_t thread_;
    pthread_mutex_t queue_mutex_;
    pthread_cond_t queue_cond_;
    queue_renderer_command_t queue_;
    atomic_bool shutdown_;
} renderer_t;

void renderer_init(renderer_t* self);
void renderer_destroy(renderer_t* self);

void renderer_redraw(renderer_t* self, syn_map_t* map, player_status_t status);
void renderer_update(renderer_t* self, coordinate_t row, coordinate_t column, map_tile_t tile);

#endif
