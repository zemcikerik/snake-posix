#include "terminal.h"
#include <stdlib.h>
#include <termbox2.h>

void terminal_update_relative_cell(const coordinates_t coordinates, const char character, const size_t index) {
    const size_t moved_index = coordinates.column_ + index;
    const int width = tb_width();

    const int x = (int) moved_index % width;
    const int y = (int) coordinates.row_ + moved_index / width;

    const int cursor_x = (x + 1) % width;
    const int cursor_y = y + (x + 1) / width;

    tb_set_cell(x, y, character, 0, 0);
    tb_set_cursor(cursor_x, cursor_y);
    tb_present();
}

void terminal_erase_relative_cell(const coordinates_t coordinates, const size_t index) {
    const size_t moved_index = coordinates.column_ + index;
    const int width = tb_width();

    const int x = (int) moved_index % width;
    const int y = (int) coordinates.row_ + moved_index / width;

    tb_set_cell(x, y, ' ', 0, 0);
    tb_set_cursor(x, y);
    tb_present();
}

bool terminal_read(const coordinates_t coordinates, char* buffer, const bool read_letters) {
    tb_set_cursor((int) coordinates.column_, (int) coordinates.row_);
    tb_present();
    size_t index = 0;

    while (true) {
        struct tb_event event;
        tb_poll_event(&event);

        if (event.type != TB_EVENT_KEY) {
            continue;
        }
        if (event.key == TB_KEY_ENTER) {
            break;
        }
        if (event.key == TB_KEY_ESC) {
            tb_hide_cursor();
            tb_present();
            return false;
        }

        if (event.key == TB_KEY_BACKSPACE || event.key == TB_KEY_BACKSPACE2) {
            if (index > 0) {
                terminal_erase_relative_cell(coordinates, --index);
            }
            continue;
        }

        if ((event.ch >= '0' && event.ch <= '9') ||
            (read_letters && ((event.ch >= 'a' && event.ch <= 'z') || (event.ch >= 'A' && event.ch <= 'Z')
                || event.ch == '.' || event.ch == '-' || event.ch == '.') || event.ch == '_')) {
            const char character = (char) event.ch;
            terminal_update_relative_cell(coordinates, character, index);
            buffer[index++] = character;
        }
    }

    buffer[index] = '\0';
    tb_hide_cursor();
    return true;
}

terminal_read_result_t terminal_read_string(const coordinates_t coordinates, char* buffer) {
    return terminal_read(coordinates, buffer, true) ? READ_SUCCESS : READ_CANCELLED;
}

terminal_read_result_t terminal_read_coordinate(const coordinates_t coordinates, coordinate_t* out_coordinate) {
    char buffer[256];

    if (!terminal_read(coordinates, buffer, false)) {
        return READ_CANCELLED;
    }

    *out_coordinate = atoi(buffer);
    return READ_SUCCESS;
}

terminal_read_result_t terminal_read_hostname(const coordinates_t coordinates, char* buffer) {
    return terminal_read(coordinates, buffer, true) ? READ_SUCCESS : READ_CANCELLED;
}

terminal_read_result_t terminal_read_port(const coordinates_t coordinates, unsigned short* out_port) {
    char buffer[256];

    if (!terminal_read(coordinates, buffer, false)) {
        return READ_CANCELLED;
    }
    if (buffer[0] == '\0') {
        return READ_EMPTY;
    }

    const int port = atoi(buffer);

    if (port < 0 || port > 65535) {
        return READ_INVALID;
    }

    *out_port = (unsigned short) port;
    return READ_SUCCESS;
}

void terminal_wait_for_key_press() {
    while (true) {
        struct tb_event event;
        tb_poll_event(&event);

        if (event.type == TB_EVENT_KEY) {
            break;
        }
    }
}

void terminal_show_notif(const char* message, const uintattr_t color) {
    tb_clear();
    tb_print(0, 0, color, 0, message);
    tb_present();
    terminal_wait_for_key_press();
}

void terminal_show_info(const char* message) {
    terminal_show_notif(message, TB_DEFAULT);
}

void terminal_show_error(const char* message) {
    terminal_show_notif(message, TB_RED);
}
