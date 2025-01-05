#ifndef INCLUDE_CLIENT_MAIN_MENU_H
#define INCLUDE_CLIENT_MAIN_MENU_H

typedef enum main_menu_result_t {
    MAIN_MENU_EXIT,
    MAIN_MENU_HOST,
    MAIN_MENU_JOIN_LOCAL,
    MAIN_MENU_JOIN_REMOTE,
} main_menu_result_t;

main_menu_result_t main_menu();

#endif
