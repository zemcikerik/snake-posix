#ifndef INCLUDE_LIBSERVER_COORDINATES_H
#define INCLUDE_LIBSERVER_COORDINATES_H

#include <stdbool.h>

typedef unsigned int coordinate_t;

typedef struct coordinates_t {
    coordinate_t row_;
    coordinate_t column_;
} coordinates_t;

bool coordinates_equal(coordinates_t lhs, coordinates_t rhs);

#endif
