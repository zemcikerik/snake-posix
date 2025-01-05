#ifndef INCLUDE_CLIENT_TERMINAL_H
#define INCLUDE_CLIENT_TERMINAL_H

#include <stdbool.h>
#include "../libsnake/coordinates.h"

typedef enum terminal_read_result_t {
    READ_CANCELLED,
    READ_INVALID,
    READ_SUCCESS,
} terminal_read_result_t;

terminal_read_result_t terminal_read_string(coordinates_t coordinates, char* buffer);
terminal_read_result_t terminal_read_port(coordinates_t coordinates, unsigned short* out_port);
void terminal_show_error(coordinates_t coordinates, const char* error);

#endif
