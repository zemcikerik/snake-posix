#ifndef INCLUDE_CLIENT_HOST_MENU_H
#define INCLUDE_CLIENT_HOST_MENU_H

typedef struct host_menu_result_t {
    char* room_name_;
} host_menu_result_t;

host_menu_result_t* host_menu();
void host_menu_free_result(host_menu_result_t* result);

#endif
