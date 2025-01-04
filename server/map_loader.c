#include "map_loader.h"
#include <stdio.h>
#include <stdlib.h>

map_template_t* map_loader_load_template_from_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");

    if (file == NULL) {
        fprintf(stderr, "Cannot open file at path: %s\n", file_path);
        return NULL;
    }

    size_t width, height;
    fscanf(file, "%zu %zu ", &width, &height);

    if (width > MAX_MAP_WIDTH || height > MAX_MAP_HEIGHT) {
        fprintf(stderr, "Dimensions of map are bigger than maximum allowed\n");
        return NULL;
    }

    map_template_t* template = malloc(sizeof(map_template_t));
    map_template_init(template, width, height);

    for (size_t i = 0; i < MAX_MAP_HEIGHT; ++i) {
        for (size_t j = 0; j < MAX_MAP_WIDTH; ++j) {
            char c;
            fscanf(file, "%c ", &c);

            if (c == '#') {
                const coordinates_t coordinates = {
                    .row_ = 0,
                    .column_ = 0
                };

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
