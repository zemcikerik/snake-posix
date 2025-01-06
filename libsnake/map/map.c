#include "map.h"
#include <stdlib.h>
#include "../rng.h"

void map_init(map_t* self, const coordinate_t width, const coordinate_t height) {
    self->width_ = width;
    self->height_ = height;

    for (coordinate_t i = 0; i < height; ++i) {
        for (coordinate_t j = 0; j < width; ++j) {
            self->tiles_[i][j].type_ = TILE_EMPTY;
        }
    }
}

void map_init_from_template(map_t* self, const map_template_t* template) {
    self->width_ = map_template_get_width(template);
    self->height_ = map_template_get_height(template);

    for (coordinate_t i = 0; i < self->height_; ++i) {
        for (coordinate_t j = 0; j < self->width_; ++j) {
            const coordinates_t coordinates = { i, j };
            self->tiles_[i][j].type_ = map_template_has_wall(template, coordinates) ? TILE_WALL : TILE_EMPTY;
        }
    }
}

coordinate_t map_get_width(const map_t* self) {
    return self->width_;
}

coordinate_t map_get_height(const map_t* self) {
    return self->height_;
}

map_tile_t map_get_tile_state(const map_t* self, const coordinates_t coordinates) {
    return self->tiles_[coordinates.row_][coordinates.column_];
}

bool map_is_tile_empty(const map_t* self, const coordinates_t coordinates) {
    return self->tiles_[coordinates.row_][coordinates.column_].type_ == TILE_EMPTY;
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

void map_set_tile_fruit(map_t* self, const coordinates_t coordinates) {
    self->tiles_[coordinates.row_][coordinates.column_].type_ = TILE_FRUIT;
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
        const coordinate_t height = map_get_height(self);

        if (direction == DIRECTION_UP) {
            coordinates.row_ = coordinates.row_ == 0 ? height - 1 : coordinates.row_ - 1;
        } else {
            coordinates.row_ = coordinates.row_ == height - 1 ? 0 : coordinates.row_ + 1;
        }
    } else {
        const coordinate_t width = map_get_width(self);

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

bool map_find_empty_neighbor(
    const map_t* self, const coordinates_t coordinates,
    coordinates_t* out_coordinates, direction_t* out_direction
) {
#define MAP_FIND_EMPTY_TILE(dir)   \
    const coordinates_t coords_##dir = map_move_in_direction(self, coordinates, dir);   \
    if (self->tiles_[coords_##dir.row_][coords_##dir.column_].type_ == TILE_EMPTY) {   \
        if (out_coordinates != NULL) {   \
            *out_coordinates = coords_##dir;   \
        }   \
        if (out_direction != NULL) {   \
            *out_direction = dir;    \
        }   \
        return true;   \
    }

    MAP_FIND_EMPTY_TILE(DIRECTION_UP);
    MAP_FIND_EMPTY_TILE(DIRECTION_DOWN);
    MAP_FIND_EMPTY_TILE(DIRECTION_LEFT);
    MAP_FIND_EMPTY_TILE(DIRECTION_RIGHT);

    return false;
}

typedef struct map_search_data_t {
    const map_t* map_;
    map_tile_predicate_t predicate_;
    coordinates_t* out_coordinates_;
    bool* visited;
} map_search_data_t;

bool map_find_random_matching_predicate_search(map_search_data_t* data, const coordinates_t coordinates) {
    const size_t index = data->map_->width_ * coordinates.row_ + coordinates.column_;

    if (data->visited[index]) {
        return false;
    }

    if (data->predicate_(data->map_, coordinates)) {
        data->out_coordinates_->row_ = coordinates.row_;
        data->out_coordinates_->column_ = coordinates.column_;
        return true;
    }

    data->visited[index] = true;
    direction_t directions[4] = { DIRECTION_UP, DIRECTION_DOWN, DIRECTION_LEFT, DIRECTION_RIGHT };

    for (size_t i = 0; i < 4; ++i) {
        const int from = rng_uniform_dist(0, 4);
        const int to = rng_uniform_dist(0, 4);

        const direction_t temp = directions[from];
        directions[from] = directions[to];
        directions[to] = temp;
    }

    for (size_t i = 0; i < 4; ++i) {
        const coordinates_t moved_coordinates = map_move_in_direction(data->map_, coordinates, directions[i]);

        if (map_find_random_matching_predicate_search(data, moved_coordinates)) {
            return true;
        }
    }

    return false;
}

bool map_find_random_matching_predicate(
    const map_t* self, const map_tile_predicate_t predicate, coordinates_t* out_coordinates
) {
    map_search_data_t search_data = {
        .map_ = self,
        .predicate_ = predicate,
        .out_coordinates_ = out_coordinates,
        .visited = calloc(self->width_ * self->height_, sizeof(bool)),
    };

    const coordinates_t start_coordinates = {
        .row_ = rng_uniform_dist(0,  (int) self->height_),
        .column_ = rng_uniform_dist(0, (int) self->width_),
    };

    const bool result = map_find_random_matching_predicate_search(&search_data, start_coordinates);
    free(search_data.visited);
    return result;
}
