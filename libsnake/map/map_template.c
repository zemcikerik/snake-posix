#include "map_template.h"

void map_template_init(map_template_t* self, const coordinate_t width, const coordinate_t height) {
    self->width_ = width;
    self->height_ = height;

    for (coordinate_t i = 0; i < height; ++i) {
        for (coordinate_t j = 0; j < width; ++j){
            self->walls_[i][j] = false;
        }
    }
}

bool map_template_has_wall(const map_template_t* self, const coordinates_t coordinates) {
    return self->walls_[coordinates.row_][coordinates.column_];
}

void map_template_set_wall(map_template_t* self, const coordinates_t coordinates) {
    self->walls_[coordinates.row_][coordinates.column_] = true;
}

coordinate_t map_template_get_width(const map_template_t* self) {
    return self->width_;
}

coordinate_t map_template_get_height(const map_template_t* self) {
    return self->height_;
}
