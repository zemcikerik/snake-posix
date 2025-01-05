#include "argument_parser.h"
#include "../libargs/args.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* argument_parser_get_str_copy(ArgParser* parser, const char* key) {
    if (!ap_found(parser, key)) {
        return NULL;
    }

    const char* string = ap_get_str_value(parser, key);
    char* buffer = malloc(strlen(string) + 1);
    strcpy(buffer, string);
    return buffer;
}

game_settings_t* argument_parser_parse(int argc, char* argv[]) {
    ArgParser* parser = ap_new_parser();

    ap_set_helptext(parser, "Hosts a snake game.");
    ap_add_int_opt(parser, "parent a", SETTING_NO_PARENT);
    ap_add_str_opt(parser, "name n", "");
    ap_add_str_opt(parser, "map m", "");
    ap_add_int_opt(parser, "width w", 0);
    ap_add_int_opt(parser, "height h", 0);

    if (!ap_parse(parser, argc, argv)) {
        ap_free(parser);
        return NULL;
    }

    if (!ap_found(parser, "name")) {
        fprintf(stderr, "Room name is required\n");
        ap_free(parser);
        return NULL;
    }

    if (!ap_found(parser, "map") && (!ap_found(parser, "width") || !ap_found(parser, "height"))) {
        fprintf(stderr, "Map settings are required\n");
        ap_free(parser);
        return NULL;
    }

    game_settings_t* settings = malloc(sizeof(game_settings_t));
    settings->room_name_ = argument_parser_get_str_copy(parser, "name");
    settings->map_path_ = argument_parser_get_str_copy(parser, "map");
    settings->width_ = ap_get_int_value(parser, "width");
    settings->height_ = ap_get_int_value(parser, "height");
    settings->parent_ = ap_get_int_value(parser, "parent");

    ap_free(parser);
    return settings;
}

void argument_parser_free(game_settings_t* settings) {
    free(settings->room_name_);

    if (settings->map_path_ != NULL) {
        free(settings->map_path_);
    }

    free(settings);
}
