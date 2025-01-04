#include "map.h"

void map_init(map_t* self, const size_t width, const size_t height) {
    self->width_ = width;
    self->height_ = height;

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            self->tiles_[i][j].type_ = TILE_EMPTY;
        }
    }
}

void map_init_from_template(map_t* self, const map_template_t* template) {
    self->width_ = map_template_get_width(template);
    self->height_ = map_template_get_height(template);

    for (size_t i = 0; i < self->height_; ++i) {
        for (size_t j = 0; j < self->width_; ++j) {
            const coordinates_t coordinates = {
                .row_ = i,
                .column_ = j,
            };

            self->tiles_[i][j].type_ = map_template_has_wall(template, coordinates) ? TILE_WALL : TILE_EMPTY;
        }
    }
}

size_t map_get_width(const map_t* self) {
    return self->width_;
}

size_t map_get_height(const map_t* self) {
    return self->height_;
}

map_tile_t map_get_tile_state(const map_t* self, const coordinates_t coordinates) {
    return self->tiles_[coordinates.row_][coordinates.column_];
}

bool map_is_tile_dead(const map_t* self, const coordinates_t coordinates) {
    return self->tiles_[coordinates.row_][coordinates.column_].type_ == TILE_DEAD;
}

void map_set_tile_empty(map_t* self, const coordinates_t coordinates) {
    self->tiles_[coordinates.row_][coordinates.column_].type_ = TILE_EMPTY;
}

void map_set_tile_player(map_t* self, const coordinates_t coordinates, const player_id_t player, const size_t order) {
    map_tile_t* tile = &self->tiles_[coordinates.row_][coordinates.column_];
    tile->type_ = TILE_PLAYER;
    tile->player_ = player;
    tile->order_ = order;
}

void map_set_tile_fruit(map_t* self, const coordinates_t coordinates, const player_id_t player) {
    map_tile_t* tile = &self->tiles_[coordinates.row_][coordinates.column_];
    tile->type_ = TILE_FRUIT;
    tile->player_ = player;
}

void map_set_tile_wall(map_t* self, const coordinates_t coordinates) {
    self->tiles_[coordinates.row_][coordinates.column_].type_ = TILE_WALL;
}

void map_set_tile_dead(map_t* self, const coordinates_t coordinates) {
    self->tiles_[coordinates.row_][coordinates.column_].type_ = TILE_DEAD;
}

void map_mark_player_as_dead(map_t* self, const player_id_t player, const coordinates_t start_coordinates) {
    const map_tile_t* tile = &self->tiles_[start_coordinates.row_][start_coordinates.column_];

    if (tile->type_ != TILE_PLAYER || tile->player_ != player) {
        return;
    }

    map_set_tile_dead(self, start_coordinates);
    map_mark_player_as_dead(self, player, map_move_in_direction(self, start_coordinates, DIRECTION_UP));
    map_mark_player_as_dead(self, player, map_move_in_direction(self, start_coordinates, DIRECTION_DOWN));
    map_mark_player_as_dead(self, player, map_move_in_direction(self, start_coordinates, DIRECTION_LEFT));
    map_mark_player_as_dead(self, player, map_move_in_direction(self, start_coordinates, DIRECTION_RIGHT));
}

coordinates_t map_move_in_direction(const map_t* self, coordinates_t coordinates, const direction_t direction) {
    if (direction == DIRECTION_UP || direction == DIRECTION_DOWN) {
        const size_t height = map_get_height(self);

        if (direction == DIRECTION_UP) {
            coordinates.row_ = coordinates.row_ == 0 ? height - 1 : coordinates.row_ - 1;
        } else {
            coordinates.row_ = coordinates.row_ == height - 1 ? 0 : coordinates.row_ + 1;
        }
    } else {
        const size_t width = map_get_width(self);

        if (direction == DIRECTION_LEFT) {
            coordinates.column_ = coordinates.column_ == 0 ? width - 1 : coordinates.column_ - 1;
        } else {
            coordinates.column_ = coordinates.column_ == width - 1 ? 0 : coordinates.column_ + 1;
        }
    }

    return coordinates;
}

bool map_find_player_neighbor_with_lowest_order(
    const map_t* self, const player_id_t player,
    const coordinates_t coordinates, coordinates_t* out_coordinates
) {
    const coordinates_t* best_coordinates = NULL;
    const map_tile_t* best_tile = NULL;

#define MAP_ASSIGN_BETTER_TILE(dir)   \
    const coordinates_t coords_##dir = map_move_in_direction(self, coordinates, dir);   \
    const map_tile_t* tile_##dir = &self->tiles_[coords_##dir.row_][coords_##dir.column_];   \
    if (tile_##dir->type_ == TILE_PLAYER && tile_##dir->player_ == player && (best_tile == NULL || best_tile->order_ > tile_##dir->order_)) {   \
        best_coordinates = &coords_##dir;   \
        best_tile = tile_##dir;   \
    }

    MAP_ASSIGN_BETTER_TILE(DIRECTION_UP);
    MAP_ASSIGN_BETTER_TILE(DIRECTION_DOWN);
    MAP_ASSIGN_BETTER_TILE(DIRECTION_LEFT);
    MAP_ASSIGN_BETTER_TILE(DIRECTION_RIGHT);

    if (best_coordinates == NULL) {
        return false;
    }

    *out_coordinates = *best_coordinates;
    return true;
}
