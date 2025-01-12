#include "socket_client.h"

#include <termbox2.h>

#include "input.h"
#include "terminal.h"
#include "../libsnake/socket.h"
#include "../libsnake/socket_message.h"
#include "../libsnake/map/syn_map.h"
#include "../libsnake/player/syn_player.h"
#include "renderer/renderer.h"

typedef enum socket_client_state_t {
    SOCKET_CLIENT_CONNECTING,
    SOCKET_CLIENT_PLAYING,
    SOCKET_CLIENT_FULL,
    SOCKET_CLIENT_GAME_ENDED,
    SOCKET_CLIENT_CONNECTION_ERROR,
} socket_client_state_t;

typedef struct socket_client_input_context_t {
    socket_t* socket_;
    _Atomic direction_t* last_direction_;
    atomic_bool* exit_;

    renderer_t* renderer_;
    syn_map_t* map_;
    _Atomic player_status_t* last_status_;
} socket_client_input_context_t;

void socket_client_resize_signal(const int how) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGWINCH);
    pthread_sigmask(how, &set, NULL);
}

void socket_client_ignore_resize_signal() {
    socket_client_resize_signal(SIG_BLOCK);
}

void socket_client_restore_resize_signal() {
    socket_client_resize_signal(SIG_UNBLOCK);
}

bool socket_client_send_magic_bytes(socket_t* socket) {
    const char buffer[4] = GAME_MAGIC_BYTES_INITIALIZER;

    socket_message_t message;
    socket_message_init_magic_bytes(&message, buffer);
    const bool result = socket_message_serialize_to_socket(&message, socket);
    socket_message_destroy(&message);

    return result;
}

void socket_client_on_arrow_press(const uint16_t arrow, void* arg) {
    socket_client_input_context_t* ctx = arg;
    direction_t new_direction;

    if (arrow == TB_KEY_ARROW_UP && *ctx->last_direction_ != DIRECTION_DOWN) {
        new_direction = DIRECTION_UP;
    } else if (arrow == TB_KEY_ARROW_DOWN && *ctx->last_direction_ != DIRECTION_UP) {
        new_direction = DIRECTION_DOWN;
    } else if (arrow == TB_KEY_ARROW_LEFT && *ctx->last_direction_ != DIRECTION_RIGHT) {
        new_direction = DIRECTION_LEFT;
    } else if (arrow == TB_KEY_ARROW_RIGHT && *ctx->last_direction_ != DIRECTION_LEFT) {
        new_direction = DIRECTION_RIGHT;
    } else {
        return;
    }

    socket_message_t message;
    socket_message_init_direction(&message, new_direction);
    socket_message_serialize_to_socket(&message, ctx->socket_);
    socket_message_destroy(&message);
}

void socket_client_on_exit_press(void* ctx) {
    *((socket_client_input_context_t*) ctx)->exit_ = true;
}

#define SEND_SIMPLE_MESSAGE(init_method)   \
    socket_message_t message;   \
    init_method(&message);      \
    socket_message_serialize_to_socket(&message, ((socket_client_input_context_t*) ctx)->socket_);   \
    socket_message_destroy(&message);

void socket_client_on_pause_press(void* ctx) {
    SEND_SIMPLE_MESSAGE(socket_message_init_pause);
}

void socket_client_on_unpause_press(void* ctx) {
    SEND_SIMPLE_MESSAGE(socket_message_init_unpause);
}

void socket_client_on_respawn_press(void* ctx) {
    SEND_SIMPLE_MESSAGE(socket_message_init_respawn);
}

void socket_client_on_resize(void* arg) {
    socket_client_input_context_t* ctx = arg;
    renderer_redraw(ctx->renderer_, ctx->map_, *ctx->last_status_);
}

void socket_client_run(const char* hostname, const in_port_t port) {
    socket_t socket;
    if (!socket_init_connect(&socket, hostname, port)) {
        terminal_show_error("Failed to connect to specified host");
        return;
    }

    if (!socket_client_send_magic_bytes(&socket)) {
        terminal_show_error("Failed to send data to specified host");
        socket_close(&socket);
        return;
    }

    syn_map_t syn_map;
    syn_map_t_init(&syn_map);
    map_init(syn_map_t_peek(&syn_map), 5, 5); // default until updated by socket

    _Atomic player_status_t last_status = PLAYER_NOT_CONNECTED;
    _Atomic direction_t last_direction = DIRECTION_UP;
    atomic_bool exit = false;

    renderer_t renderer;
    renderer_init(&renderer);

    socket_client_input_context_t input_context = {
        .socket_ = &socket,
        .last_direction_ = &last_direction,
        .exit_ = &exit,

        .map_ = &syn_map,
        .renderer_ = &renderer,
        .last_status_ = &last_status,
    };
    input_callbacks_t input_callbacks = {
        .on_arrow_press = socket_client_on_arrow_press,
        .on_exit_press = socket_client_on_exit_press,
        .on_pause_press = socket_client_on_pause_press,
        .on_unpause_press = socket_client_on_unpause_press,
        .on_respawn_press = socket_client_on_respawn_press,
        .on_resize = socket_client_on_resize,
        .ctx_ = &input_context,
    };
    input_t input;
    input_init(&input, &input_callbacks);
    socket_client_ignore_resize_signal();

    socket_client_state_t state = SOCKET_CLIENT_CONNECTING;

    while (state == SOCKET_CLIENT_CONNECTING || state == SOCKET_CLIENT_PLAYING && !exit) {
        socket_message_t message;

        if (!socket_message_deserialize_from_socket(&socket, &message)) {
            state = SOCKET_CLIENT_CONNECTION_ERROR;
            break;
        }

        if (state != SOCKET_CLIENT_CONNECTING && state != SOCKET_CLIENT_PLAYING) {
            break;
        }

        if (message.type_ == SOCKET_MESSAGE_GAME_FULL) {
            state = SOCKET_CLIENT_FULL;
            socket_message_destroy(&message);
            break;
        }

        if (message.type_ == SOCKET_MESSAGE_GAME_END) {
            state = SOCKET_CLIENT_GAME_ENDED;
            socket_message_destroy(&message);
            break;
        }

        if (message.type_ == SOCKET_MESSAGE_CONNECTED) {
            state = SOCKET_CLIENT_PLAYING;
            socket_message_destroy(&message);
            continue;
        }

        if (message.type_ == SOCKET_MESSAGE_MAP) {
            map_t* map = syn_map_t_acquire(&syn_map);
            socket_message_deserialize_to_map(&message, map);
            syn_map_t_release(&syn_map);

            renderer_redraw(&renderer, &syn_map, last_status);
            socket_message_destroy(&message);
            continue;
        }

        if (message.type_ == SOCKET_MESSAGE_TILE_CHANGE) {
            const coordinates_t coordinates = { message.data_.tile_change_->row_, message.data_.tile_change_->column_ };

            map_t* map = syn_map_t_acquire(&syn_map);
            map_set_tile_state(map, coordinates, message.data_.tile_change_->tile_);
            syn_map_t_release(&syn_map);

            renderer_update(&renderer, coordinates.row_, coordinates.column_, message.data_.tile_change_->tile_);
            socket_message_destroy(&message);
            continue;
        }

        if (message.type_ == SOCKET_MESSAGE_PLAYER_STATE) {
            const bool status_changed = message.data_.player_state_->status_ != last_status;
            last_status = message.data_.player_state_->status_;
            last_direction = message.data_.player_state_->direction_;
            socket_message_destroy(&message);

            if (status_changed) {
                renderer_redraw(&renderer, &syn_map, last_status);
            }

            continue;
        }

        if (message.type_ == SOCKET_MESSAGE_GAME_FULL) {
            state = SOCKET_CLIENT_FULL;
            socket_message_destroy(&message);
        }

        fprintf(stderr, "Received unexpected message type: %d\n", message.type_);
        state = SOCKET_CLIENT_CONNECTION_ERROR;
        socket_message_destroy(&message);
    }

    if (exit) {
        socket_message_t message;
        socket_message_init_disconnect(&message);
        socket_message_serialize_to_socket(&message, &socket);
        socket_message_destroy(&message);
    }

    socket_client_restore_resize_signal();
    input_destroy(&input);
    renderer_destroy(&renderer);
    syn_map_t_destroy(&syn_map);
    socket_close(&socket);

    if (state == SOCKET_CLIENT_FULL) {
        terminal_show_error("Game is full!");
    } else if (state == SOCKET_CLIENT_GAME_ENDED) {
        terminal_show_info("Game has ended.");
    } else if (state == SOCKET_CLIENT_CONNECTION_ERROR) {
        terminal_show_error("There was an error with connection to the server.");
    }
}
