#ifndef INCLUDE_LIBSERVER_COORDINATES_H
#define INCLUDE_LIBSERVER_COORDINATES_H

#include <stddef.h>
#include <stdbool.h>

typedef struct coordinates_t {
    size_t row_;
    size_t column_;
} coordinates_t;

bool coordinates_equal(coordinates_t lhs, coordinates_t rhs);

#endif
