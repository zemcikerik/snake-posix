#include "map_menu.h"
#include <termbox2.h>

const uintattr_t FOREGROUND_COLORS[6] = {
    TB_RED,
    TB_GREEN,
    TB_YELLOW,
    TB_BLUE,
    TB_MAGENTA,
    TB_CYAN,
};

#define FOREGROUND_COLORS_LENGTH 6

void map_menu_init(map_menu_t* self, syn_map_t* map) {
    self->map_ = map;
    map_menu_redraw(self);
}

char map_menu_get_tile_character(const map_tile_type_t type) {
    switch (type) {
        case TILE_PLAYER:
            return '*';
        case TILE_WALL:
            return '#';
        case TILE_FRUIT:
            return 'O';
        default:
            return ' ';
    }
}

void map_menu_update(const coordinates_t coordinates, const map_tile_t tile) {
    const char character = map_menu_get_tile_character(tile.type_);
    const uintattr_t color = tile.type_ == TILE_PLAYER
        ? FOREGROUND_COLORS[tile.player_ % FOREGROUND_COLORS_LENGTH]
        : TB_DEFAULT;

    tb_set_cell((int) coordinates.column_ + 1, (int) coordinates.row_ + 1, character, color, 0);
}

void map_menu_update_finished() {
    tb_present();
}

void map_menu_redraw(map_menu_t* self) {
    tb_clear();

    const map_t* map = syn_map_t_acquire(self->map_);
    const size_t width = map_get_width(map);
    const size_t height = map_get_height(map);

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            const coordinates_t coordinates = {
                .row_ = i,
                .column_ = j
            };

            map_menu_update(coordinates, map_get_tile_state(map, coordinates));
        }
    }

    syn_map_t_release(self->map_);

    for (size_t i = 0; i < width + 2; ++i) {
        tb_set_cell((int) i, 0, '#', TB_WHITE, TB_RED);
        tb_set_cell((int) i, (int) height + 1, '#', TB_WHITE, TB_RED);
    }

    for (size_t i = 1; i < height + 1; ++i) {
        tb_set_cell(0, (int) i, '#', TB_WHITE, TB_RED);
        tb_set_cell((int) width + 1, (int) i, '#', TB_WHITE, TB_RED);
    }

    tb_present();
}

