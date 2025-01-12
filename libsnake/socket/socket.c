#include "socket.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

bool socket_init_connect(socket_t* self, const char* host, const in_port_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char port_str[6];
    sprintf(port_str, "%hu", port);

    struct addrinfo* found_addresses;
    const int result = getaddrinfo(host, port_str, &hints, &found_addresses);
    if (result != 0) {
        fprintf(stderr, "Failed to find server addr info: %s\n", gai_strerror(result));
        return false;
    }

    for (struct addrinfo* info = found_addresses; info != NULL; info = info->ai_next) {
        const int fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

        if (fd == -1) {
            continue;
        }

        if (connect(fd, info->ai_addr, info->ai_addrlen) == -1) {
            close(fd);
            continue;
        }

        freeaddrinfo(found_addresses);
        self->fd_ = fd;
        return true;
    }

    perror("Failed to connect to server");
    freeaddrinfo(found_addresses);
    return false;
}

bool socket_init_listen(socket_t* self, const in_port_t port) {
    self->fd_ = socket(AF_INET, SOCK_STREAM, 0);

    if (self->fd_ == -1) {
        perror("Failed to create socket");
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(self->fd_, (const struct sockaddr*) &addr, sizeof(struct sockaddr_in)) == -1) {
        perror("Failed to bind socket");
        close(self->fd_);
        return false;
    }

    if (listen(self->fd_, min(8, SOMAXCONN)) == -1) {
        perror("Failed to listen on socket");
        close(self->fd_);
        return false;
    }

    return true;
}

bool socket_accept(socket_t* self, socket_t* out_socket) {
    out_socket->fd_ = accept(self->fd_, NULL, NULL);

    if (out_socket->fd_ != -1) {
        return true;
    }

    if (errno == EINTR) {
        return false;
    }

    perror("Failed to accept client");
    return false;
}

bool socket_read(socket_t* self, char* buffer, const size_t buffer_length) {
    size_t read_bytes = 0;

    while (read_bytes < buffer_length) {
        const ssize_t r = read(self->fd_, buffer, buffer_length - read_bytes);

        if (r == -1) {
            return false;
        }

        read_bytes += r;
    }

    return true;
}

bool socket_write(socket_t* self, const char* buffer, const size_t buffer_length) {
    size_t written_bytes = 0;

    while (written_bytes < buffer_length) {
        const ssize_t w = write(self->fd_, buffer, buffer_length - written_bytes);

        if (w == -1) {
            return false;
        }

        written_bytes += w;
    }

    return true;
}

void socket_close(socket_t* self) {
    close(self->fd_);
}
