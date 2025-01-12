#include "client_connection_handler.h"
#include <unistd.h>
#include "../libsnake/socket/socket_message.h"

bool client_connection_handler_verify_magic_bytes(socket_t* socket) {
    socket_message_t message;

    if (!socket_message_deserialize_from_socket(socket, &message)) {
        return false;
    }

    const bool result = message.type_ == SOCKET_MESSAGE_MAGIC_BYTES && GAME_MAGIC_BYTES_CORRECT(message.data_.magic_bytes_->bytes_);
    socket_message_destroy(&message);
    return result;
}

void* client_connection_handler_main(void* arg) {
    client_connection_handler_t* self = arg;

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIG_DESTROYED);
    pthread_sigmask(SIG_SETMASK, &set, NULL);

    while (!self->shutdown_) {
        socket_t connection;

        if (!socket_accept(&self->socket_, &connection)) {
            break;
        }

        if (self->shutdown_) {
            socket_close(&connection);
            break;
        }

        if (!client_connection_handler_verify_magic_bytes(&connection)) {
            socket_close(&connection);
            continue;
        }

        if (self->connections_capacity_ == self->connections_size_) {
            self->connections_capacity_ *= 2;
            self->connections_ = realloc(self->connections_, self->connections_capacity_ * sizeof(client_connection_t));
        }

        client_connection_init(&self->connections_[self->connections_size_++], connection, self->game_state_);
    }

    for (size_t i = 0; i < self->connections_size_; ++i) {
        client_connection_destroy(&self->connections_[i]);
    }

    socket_close(&self->socket_);
    return NULL;
}

void client_connection_handler_init(client_connection_handler_t* self, game_state_t* game_state, const socket_t socket) {
    self->game_state_ = game_state;
    self->socket_ = socket;
    self->connections_ = malloc(4 * sizeof(client_connection_t));
    self->connections_size_ = 0;
    self->connections_capacity_ = 4;
    self->shutdown_ = false;
    pthread_create(&self->thread_, NULL, client_connection_handler_main, self);
}

void client_connection_handler_destroy(client_connection_handler_t* self) {
    self->shutdown_ = true;
    pthread_kill(self->thread_, SIG_DESTROYED);
    pthread_join(self->thread_, NULL);
    free(self->connections_);
}
