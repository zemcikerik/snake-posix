#ifndef INCLUDE_LIBSNAKE_MAP_TEMPLATE_H
#define INCLUDE_LIBSNAKE_MAP_TEMPLATE_H

#include <stdbool.h>
#include "../constants.h"
#include "../coordinates.h"

typedef struct map_template_t {
    bool walls_[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
    coordinate_t width_;
    coordinate_t height_;
} map_template_t;

void map_template_init(map_template_t* self, coordinate_t width, coordinate_t height);

bool map_template_has_wall(const map_template_t* self, coordinates_t coordinates);
void map_template_set_wall(map_template_t* self, coordinates_t coordinates);

coordinate_t map_template_get_width(const map_template_t* self);
coordinate_t map_template_get_height(const map_template_t* self);

#endif
