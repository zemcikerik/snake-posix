#ifndef INCLUDE_SERVER_MAP_LOADER_H
#define INCLUDE_SERVER_MAP_LOADER_H

#include "../libsnake/map/map_template.h"

map_template_t* map_loader_create_empty_template(size_t width, size_t height);
map_template_t* map_loader_load_template_from_file(const char* file_path);
void map_loader_free_template(map_template_t* template);

#endif
