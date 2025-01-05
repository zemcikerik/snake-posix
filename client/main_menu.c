#include "main_menu.h"
#include "../libtermbox2/termbox2.h"

main_menu_result_t main_menu() {
    const char* logo =
        "     _______..__   __.      ___       __  ___  _______\n"
        "    /       ||  \\ |  |     /   \\     |  |/  / |   ____|\n"
        "   |   (----`|   \\|  |    /  ^  \\    |  '  /  |  |__\n"
        "    \\   \\    |  . `  |   /  /_\\  \\   |    <   |   __|\n"
        ".----)   |   |  |\\   |  /  _____  \\  |  .  \\  |  |____\n"
        "|_______/    |__| \\__| /__/     \\__\\ |__|\\__\\ |_______|";

    tb_print(0, 2, TB_GREEN, 0, logo);
    tb_print(17, 9, TB_RED, 0, "Erik Zemcik 5ZYI33");
    tb_print(0, 12, 0, 0, "1. Host new game");
    tb_print(0, 13, 0, 0, "2. Join local game");
    tb_print(0, 14, 0, 0, "3. Join remote game");
    tb_print(0, 15, 0, 0, "4. Exit");
    tb_present();

    while (1) {
        struct tb_event event;
        tb_poll_event(&event);

        if (event.type != TB_EVENT_KEY) {
            continue;
        }

        if (event.key == TB_KEY_ESC || event.key == TB_KEY_CTRL_C || event.ch == '4') {
            return MAIN_MENU_EXIT;
        }
        if (event.ch == '1') {
            return MAIN_MENU_HOST;
        }
        if (event.ch == '2') {
            return MAIN_MENU_JOIN_LOCAL;
        }
        if (event.ch == '3') {
            return MAIN_MENU_JOIN_REMOTE;
        }
    }
}
