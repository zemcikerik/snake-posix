#include "map_loader.h"
#include <stdio.h>
#include <stdlib.h>

map_template_t* map_loader_create_empty_template(const coordinate_t width, const coordinate_t height) {
    map_template_t* template = malloc(sizeof(map_template_t));
    map_template_init(template, width, height);
    return template;
}

map_template_t* map_loader_load_template_from_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");

    if (file == NULL) {
        fprintf(stderr, "Cannot open file at path: %s\n", file_path);
        return NULL;
    }

    coordinate_t width, height;
    fscanf(file, "%u %u ", &width, &height);

    if (width < MIN_MAP_WIDTH || height < MIN_MAP_HEIGHT) {
        fprintf(stderr, "Dimensions of map are smaller than minimum allowed\n");
        return NULL;
    }

    if (width > MAX_MAP_WIDTH || height > MAX_MAP_HEIGHT) {
        fprintf(stderr, "Dimensions of map are bigger than maximum allowed\n");
        return NULL;
    }

    map_template_t* template = map_loader_create_empty_template(width, height);

    for (coordinate_t i = 0; i < height; ++i) {
        for (coordinate_t j = 0; j < width; ++j) {
            char c;
            fscanf(file, "%c ", &c);

            if (c == '#') {
                const coordinates_t coordinates = { i, j };
                map_template_set_wall(template, coordinates);
            }
        }
    }

    fclose(file);
    return template;
}

void map_loader_free_template(map_template_t* template) {
    free(template);
}
