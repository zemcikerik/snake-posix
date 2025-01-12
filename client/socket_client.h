#ifndef INCLUDE_CLIENT_SOCKET_CLIENT_H
#define INCLUDE_CLIENT_SOCKET_CLIENT_H

#include <netdb.h>

void socket_client_run(const char* hostname, in_port_t port);

#endif
