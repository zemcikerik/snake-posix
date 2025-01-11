#include "client_connection_acceptor.h"

#include "../libsnake/socket_message.h"

bool client_connection_acceptor_handle_magic_bytes(socket_t* socket) {
    socket_message_t message;

    if (!socket_message_deserialize_from_socket(socket, &message)) {
        return false;
    }

    if (message.type_ != SOCKET_MESSAGE_MAGIC_BYTES) {
        return false;
    }

    // todo check
    return true;
}

void* client_connection_acceptor_main(void* arg) {
    client_connection_acceptor_t* self = arg;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    while (!self->shutdown_) {
        socket_t connection;

        if (!socket_accept(&self->socket_, &connection)) {
            return NULL;
        }

        if (self->shutdown_) {
            socket_close(&connection);
            return NULL;
        }

        if (!client_connection_acceptor_handle_magic_bytes(&connection)) {
            socket_close(&connection);
            continue;
        }

        if (self->connections_capacity_ == self->connections_size_) {
            self->connections_capacity_ *= 2;
            self->connections_ = realloc(self->connections_, self->connections_capacity_ * sizeof(client_connection_t));
        }

        client_connection_init(&self->connections_[self->connections_size_++], connection, self->game_state_);
    }

    return NULL;
}

void client_connection_handler_init(client_connection_acceptor_t* self, game_state_t* game_state, const socket_t socket) {
    self->game_state_ = game_state;
    self->socket_ = socket;
    self->connections_ = malloc(4 * sizeof(client_connection_t));
    self->connections_size_ = 0;
    self->connections_capacity_ = 4;
    self->shutdown_ = false;
    pthread_create(&self->thread_, NULL, client_connection_acceptor_main, self);
}

void client_connection_handler_destroy(client_connection_acceptor_t* self) {
    self->shutdown_ = true;
    pthread_kill(self->thread_, SIGUSR2);
    pthread_join(self->thread_, NULL);

    for (size_t i = 0; i < self->connections_size_; ++i) {
        client_connection_destroy(&self->connections_[i]);
    }

    free(self->connections_);
}
