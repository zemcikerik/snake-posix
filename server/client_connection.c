#include "client_connection.h"
#include <unistd.h>
#include "../libsnake/socket_message.h"

void client_connection_listen_for_signal() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void client_connection_shutdown(client_connection_t* self) {
    if (!self->shutdown_) {
        return;
    }

    self->shutdown_ = true;
    pthread_kill(self->receive_thread_, SIGUSR2);
    pthread_kill(self->send_thread_, SIGUSR2);

}

bool client_connection_send_map(client_connection_t* self) {
    socket_message_t message;

    const map_t* map = syn_map_t_acquire(&self->game_state_->map_);
    socket_message_init_map(&message, map);
    syn_map_t_release(&self->game_state_->map_);

    const bool result = socket_message_serialize_to_socket(&message, &self->socket_);
    socket_message_destroy(&message);
    return result;
}

bool client_connection_send_changelog(client_connection_t* self) {
    size_t size;
    change_t* changes = syn_changelog_copy(&self->game_state_->changelog_, &size);

    if (changes == NULL) {
        return true;
    }

    for (size_t i = 0; i < size; ++i) {
        if (changes[i].type_ != CHANGE_TILE_STATE) {
            fprintf(stderr, "Unsupported tile type\n");
            exit(EXIT_FAILURE);
        }

        socket_message_t message;
        const change_tile_state_data_t* data = &changes->data_.change_tile_state_data_;
        socket_message_init_tile_change(&message, data->coordinates_.row_, data->coordinates_.column_, data->tile_data_);

        const bool result = socket_message_serialize_to_socket(&message, &self->socket_);
        socket_message_destroy(&message);

        if (!result) {
            syn_changelog_free_copy(changes);
            return false;
        }
    }

    syn_changelog_free_copy(changes);
    return true;
}

#define SHUTDOWN_IF_FAIL(statement) if (!statement) client_connection_shutdown(self)

void* client_connection_send_thread_main(void* arg) {
    client_connection_t* self = arg;
    client_connection_listen_for_signal();

    player_id_t player_id;
    if (!player_manager_register(&self->game_state_->player_manager_, &player_id)) {
        socket_message_t message;
        socket_message_init_game_end(&message);
        socket_message_serialize_to_socket(&message, &self->socket_);
        socket_message_destroy(&message);
        socket_close(&self->socket_);
        return NULL;
    }

    ticker_observer_t observer;
    ticker_observer_init(&observer, &self->game_state_->ticker_);
    SHUTDOWN_IF_FAIL(client_connection_send_map(self));

    while (!self->shutdown_) {
        const size_t ticks_passed = ticker_observer_wait_for_next_tick(&observer);

        if (self->shutdown_ || self->game_state_->game_ended_) {
            break;
        }

        if (ticks_passed > 1) {
            SHUTDOWN_IF_FAIL(client_connection_send_map(self));
        } else {
            SHUTDOWN_IF_FAIL(client_connection_send_changelog(self));
        }

        if (self->shutdown_) {
            break;
        }
    }

    if (self->game_state_->game_ended_) {
        socket_message_t message;
        socket_message_init_game_end(&message);
        socket_message_serialize_to_socket(&message, &self->socket_);
        socket_message_destroy(&message);
    }

    client_connection_shutdown(self);
    socket_close(&self->socket_);
    return NULL;
}

void* client_connection_receive_thread_main(void* args) {
    client_connection_t* self = args;
    client_connection_listen_for_signal();

    while (!self->shutdown_) {
        socket_message_t message;

        if (!socket_message_deserialize_from_socket(&self->socket_, &message) || !self->shutdown_) {
            break;
        }
    }

    return NULL;
}

void client_connection_init(client_connection_t* self, socket_t socket, game_state_t* game_state) {
    self->game_state_ = game_state;
    self->socket_ = socket;
    self->shutdown_ = false;
    pthread_create(&self->receive_thread_, NULL, client_connection_receive_thread_main, self);
    pthread_create(&self->send_thread_, NULL, client_connection_send_thread_main, self);
}

void client_connection_destroy(client_connection_t* self) {
    client_connection_shutdown(self);
    pthread_join(self->receive_thread_, NULL);
    pthread_join(self->send_thread_, NULL);
}
