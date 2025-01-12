#ifndef INCLUDE_SERVER_CLIENT_CONNECTION_H
#define INCLUDE_SERVER_CLIENT_CONNECTION_H

#include <stdatomic.h>
#include "../libsnake/game_state.h"
#include "../libsnake/socket.h"

typedef struct client_connection_t {
    pthread_t receive_thread_;
    pthread_t send_thread_;
    game_state_t* game_state_;
    player_id_t player_id_;
    socket_t socket_;
    atomic_bool shutdown_;
} client_connection_t;

void client_connection_init(client_connection_t* self, socket_t socket, game_state_t* game_state);
void client_connection_destroy(client_connection_t* self);

#endif
