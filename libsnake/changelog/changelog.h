#ifndef INCLUDE_LIBSNAKE_CHANGELOG_H
#define INCLUDE_LIBSNAKE_CHANGELOG_H

#include "../coordinates.h"
#include "../map/map.h"

typedef enum change_type_t {
    CHANGE_TILE_STATE
} change_type_t;

typedef struct change_tile_state_data_t {
    coordinates_t coordinates_;
    map_tile_t tile_data_;
} change_tile_state_data_t;

typedef union change_data_t {
    change_tile_state_data_t change_tile_state_data_;
} change_data_t;

typedef struct change_t {
    change_type_t type_;
    change_data_t data_;
} change_t;

typedef struct changelog_t {
    change_t changes_[MAX_CHANGELOG_SIZE];
    size_t size_;
} changelog_t;

typedef void (*changelog_for_each_fn)(changelog_t* changelog, change_t* change, void* ctx);

void changelog_init(changelog_t* self);
void changelog_clear(changelog_t* self);
void changelog_push_tile_state_change(changelog_t* self, coordinates_t coordinates, map_tile_t map_tile);

#endif
