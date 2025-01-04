#include "coordinates.h"

bool coordinates_equal(const coordinates_t lhs, const coordinates_t rhs) {
    return lhs.row_ == rhs.row_ && lhs.column_ == rhs.column_;
}
