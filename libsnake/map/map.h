#ifndef INCLUDE_LIBSNAKE_MAP_H
#define INCLUDE_LIBSNAKE_MAP_H

#include <stdbool.h>
#include <stddef.h>

#include "map_template.h"
#include "../constants.h"
#include "../coordinates.h"
#include "../direction.h"
#include "../player/player.h"

typedef enum map_tile_type_t {
    TILE_EMPTY,
    TILE_PLAYER,
    TILE_FRUIT,
    TILE_WALL,
    TILE_DEAD,
} map_tile_type_t;

typedef struct map_tile_t {
    size_t order_;
    map_tile_type_t type_;
    player_id_t player_;
} map_tile_t;

typedef struct map_t {
    map_tile_t tiles_[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
    size_t width_;
    size_t height_;
} map_t;

void map_init(map_t* self, size_t width, size_t height);
void map_init_from_template(map_t* self, const map_template_t* template);

size_t map_get_width(const map_t* self);
size_t map_get_height(const map_t* self);

map_tile_t map_get_tile_state(const map_t* self, coordinates_t coordinates);
bool map_is_tile_dead(const map_t* self, coordinates_t coordinates);

void map_set_tile_empty(map_t* self, coordinates_t coordinates);
void map_set_tile_player(map_t* self, coordinates_t coordinates, player_id_t player, size_t order);
void map_set_tile_fruit(map_t* self, coordinates_t coordinates, player_id_t player);
void map_set_tile_wall(map_t* self, coordinates_t coordinates);
void map_set_tile_dead(map_t* self, coordinates_t coordinates);

void map_mark_player_as_dead(map_t* self, player_id_t player, coordinates_t start_coordinates);

coordinates_t map_move_in_direction(const map_t* self, coordinates_t coordinates, direction_t direction);

bool map_find_player_neighbor_with_lowest_order(
    const map_t* self, player_id_t player,
    coordinates_t coordinates, coordinates_t* out_coordinates
);

#endif
