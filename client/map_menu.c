#include "map_menu.h"
#include <termbox2.h>

#define FOREGROUND_COLORS_LENGTH 6
const uintattr_t FOREGROUND_COLORS[FOREGROUND_COLORS_LENGTH] = {
    TB_RED,
    TB_GREEN,
    TB_YELLOW,
    TB_BLUE,
    TB_MAGENTA,
    TB_CYAN,
};

void* map_menu_input_main(void* arg) {
    map_menu_t* self = arg;

    while (true) {
        struct tb_event event;
        const int peek_result = tb_peek_event(&event, 5);

        if (self->destroyed_) {
            break;
        }

        if (peek_result != TB_ERR_NO_EVENT && peek_result != TB_ERR_POLL && event.type == TB_EVENT_KEY) {
            self->callback_(self, event.key, event.ch, self->callback_ctx_);
        }
    }

    return NULL;
}

void map_menu_init(map_menu_t* self, syn_map_t* map, const map_menu_key_callback_t callback, void* ctx) {
    self->map_ = map;
    self->callback_ = callback;
    self->callback_ctx_ = ctx;
    self->destroyed_ = false;
    pthread_create(&self->input_thread_, NULL, map_menu_input_main, self);
}

void map_menu_destroy(map_menu_t* self) {
    self->destroyed_ = true;
    pthread_join(self->input_thread_, NULL);
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

void map_menu_update_end() {
    tb_present();
}

void map_menu_redraw(map_menu_t* self, const player_status_t player_status) {
    tb_clear();

    const map_t* map = syn_map_t_acquire(self->map_);
    const coordinate_t width = map_get_width(map);
    const coordinate_t height = map_get_height(map);

    for (coordinate_t i = 0; i < height; ++i) {
        for (coordinate_t j = 0; j < width; ++j) {
            const coordinates_t coordinates = { i, j };
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

    tb_print(0, (int) height + 4, 0, 0, "Controls:");
    tb_print(1, (int) height + 5, 0, 0, "-");

#define PRINT_CONTROL(key, description, index)   \
    tb_print(1, (int) height + 5 + index, 0, 0, "-");   \
    tb_print(3, (int) height + 5 + index, TB_YELLOW, 0, key);   \
    tb_print(5 + strlen(key), (int) height + 5 + index, 0, 0, "- " description);   \

    PRINT_CONTROL("ESC", "leave the game", 0);

    if (player_status == PLAYER_DEAD) {
        PRINT_CONTROL("R", "respawn", 1);
    } else if (player_status == PLAYER_PLAYING) {
        PRINT_CONTROL("arrows", "control the snake movement", 1);
        PRINT_CONTROL("P", "pause the game", 2);
    } else if (player_status == PLAYER_PAUSED) {
        PRINT_CONTROL("U", "unpause the game", 1);
    }

    tb_present();
}

