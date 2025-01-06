#ifndef INCLUDE_CLIENT_TERMINAL_H
#define INCLUDE_CLIENT_TERMINAL_H

#include "../libsnake/coordinates.h"

typedef enum terminal_read_result_t {
    READ_CANCELLED,
    READ_SUCCESS,
} terminal_read_result_t;

terminal_read_result_t terminal_read_string(coordinates_t coordinates, char* buffer);
terminal_read_result_t terminal_read_coordinate(coordinates_t coordinates, coordinate_t* out_coordinate);
void terminal_show_info(const char* message);
void terminal_show_error(const char* message);

#endif
