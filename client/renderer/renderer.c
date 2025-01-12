#include "renderer.h"
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

char renderer_get_tile_character(const map_tile_type_t type) {
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


void renderer_update_tile(const coordinate_t row, const coordinate_t column, const map_tile_t tile) {
    const char character = renderer_get_tile_character(tile.type_);
    const uintattr_t color = tile.type_ == TILE_PLAYER
        ? FOREGROUND_COLORS[tile.player_ % FOREGROUND_COLORS_LENGTH]
        : TB_DEFAULT;

    tb_set_cell((int) column + 1, (int) row + 1, character, color, 0);
}

void renderer_redraw_screen(syn_map_t* syn_map, const player_status_t status) {
    tb_clear();

    const map_t* map = syn_map_t_acquire(syn_map);
    const coordinate_t width = map_get_width(map);
    const coordinate_t height = map_get_height(map);

    for (coordinate_t i = 0; i < height; ++i) {
        for (coordinate_t j = 0; j < width; ++j) {
            const coordinates_t coordinates = { i, j };
            renderer_update_tile(i, j, map_get_tile_state(map, coordinates));
        }
    }

    syn_map_t_release(syn_map);

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

    if (status == PLAYER_DEAD) {
        PRINT_CONTROL("R", "respawn", 1);
    } else if (status == PLAYER_PLAYING) {
        PRINT_CONTROL("arrows", "control the snake movement", 1);
        PRINT_CONTROL("P", "pause the game", 2);
    } else if (status == PLAYER_PAUSED) {
        PRINT_CONTROL("U", "unpause the game", 1);
    }

    tb_present();
}

void* renderer_main(void* arg) {
    renderer_t* self = arg;

    while (!self->shutdown_) {
        pthread_mutex_lock(&self->queue_mutex_);

        while (!self->shutdown_ && queue_renderer_command_t_is_empty(&self->queue_)) {
            pthread_cond_wait(&self->queue_cond_, &self->queue_mutex_);
        }

        if (self->shutdown_) {
            pthread_mutex_unlock(&self->queue_mutex_);
            break;
        }

        renderer_command_t command;
        queue_renderer_command_t_pop(&self->queue_, &command);

        if (command.type_ == RENDERER_COMMAND_REDRAW) {
            renderer_redraw_screen(command.data_.redraw_.map_, command.data_.redraw_.status_);
            pthread_mutex_unlock(&self->queue_mutex_);
            continue;
        }

        do {
            renderer_update_tile(command.data_.update_.row_, command.data_.update_.column_, command.data_.update_.tile_);
        } while (queue_renderer_command_t_pop(&self->queue_, &command));

        tb_present();
        pthread_mutex_unlock(&self->queue_mutex_);
    }

    return NULL;
}

void renderer_init(renderer_t* self) {
    queue_renderer_command_t_init(&self->queue_);
    pthread_mutex_init(&self->queue_mutex_, NULL);
    pthread_cond_init(&self->queue_cond_, NULL);
    self->shutdown_ = false;
    pthread_create(&self->thread_, NULL, renderer_main, self);
}

void renderer_destroy(renderer_t* self) {
    self->shutdown_ = true;
    pthread_cond_signal(&self->queue_cond_);
    pthread_join(self->thread_, NULL);
    queue_renderer_command_t_destroy(&self->queue_);
}

void renderer_redraw(renderer_t* self, syn_map_t* map, const player_status_t status) {
    renderer_command_t command;
    command.type_ = RENDERER_COMMAND_REDRAW;
    command.data_.redraw_.map_ = map;
    command.data_.redraw_.status_ = status;

    pthread_mutex_lock(&self->queue_mutex_);
    queue_renderer_command_t_clear(&self->queue_);
    queue_renderer_command_t_push(&self->queue_, &command);
    pthread_cond_signal(&self->queue_cond_);
    pthread_mutex_unlock(&self->queue_mutex_);
}

void renderer_update(renderer_t* self, const coordinate_t row, const coordinate_t column, const map_tile_t tile) {
    renderer_command_t command;
    command.type_ = RENDERER_COMMAND_UPDATE;
    command.data_.update_.row_ = row;
    command.data_.update_.column_ = column;
    command.data_.update_.tile_ = tile;

    pthread_mutex_lock(&self->queue_mutex_);
    queue_renderer_command_t_push(&self->queue_, &command);
    pthread_cond_signal(&self->queue_cond_);
    pthread_mutex_unlock(&self->queue_mutex_);
}
