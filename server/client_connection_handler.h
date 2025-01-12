#ifndef INCLUDE_SERVER_CLIENT_CONNECTION_ACCEPTOR_H
#define INCLUDE_SERVER_CLIENT_CONNECTION_ACCEPTOR_H

#include <stdatomic.h>
#include <pthread.h>
#include "client_connection.h"
#include "../libsnake/game_state.h"

typedef struct client_connection_handler_t {
    pthread_t thread_;
    game_state_t* game_state_;
    client_connection_t* connections_;
    size_t connections_size_;
    size_t connections_capacity_;
    socket_t socket_;
    atomic_bool shutdown_;
} client_connection_handler_t;

void client_connection_handler_init(client_connection_handler_t* self, game_state_t* game_state, socket_t socket);
void client_connection_handler_destroy(client_connection_handler_t* self);

#endif
