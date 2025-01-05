#ifndef INCLUDE_SERVER_MAP_SETTINGS_H
#define INCLUDE_SERVER_MAP_SETTINGS_H

#include <stdbool.h>
#include <stddef.h>

typedef struct map_settings_t {
    union {
        const char* path_;
        struct {
            size_t width_;
            size_t height_;
        };
    };
    bool has_path_;
} map_settings_t;

#endif
