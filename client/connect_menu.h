#ifndef INCLUDE_CLIENT_CONNECT_MENU_H
#define INCLUDE_CLIENT_CONNECT_MENU_H

#include <stdbool.h>

bool connect_menu_local(char* out_room_name);
bool connect_menu_remote(char* out_hostname, unsigned short* out_port);

#endif
