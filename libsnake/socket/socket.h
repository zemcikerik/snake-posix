#ifndef INCLUDE_LIBSNAKE_SOCKET_H
#define INCLUDE_LIBSNAKE_SOCKET_H

#include <stdbool.h>
#include <netinet/in.h>

typedef struct socket_t {
    int fd_;
} socket_t;

bool socket_init_connect(socket_t* self, const char* host, in_port_t port);
bool socket_init_listen(socket_t* self, in_port_t port);
bool socket_accept(socket_t* self, socket_t* out_socket);
bool socket_read(socket_t* self, char* buffer, size_t buffer_length);
bool socket_write(socket_t* self, const char* buffer, size_t buffer_length);
void socket_close(socket_t* self);

#endif
