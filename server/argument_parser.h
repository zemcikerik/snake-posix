#ifndef INCLUDE_SERVER_ARGUMENT_PARSER_H
#define INCLUDE_SERVER_ARGUMENT_PARSER_H

#include "../libsnake/game_settings.h"

game_settings_t* argument_parser_parse(int argc, char* argv[]);
void argument_parser_free(game_settings_t* settings);

#endif
