#ifndef INCLUDE_LIBSNAKE_GAME_STATE_H
#define INCLUDE_LIBSNAKE_GAME_STATE_H

#include "ticker.h"
#include "changelog/syn_changelog.h"
#include "map/map_template.h"
#include "map/syn_map.h"
#include "player/player_manager.h"

typedef struct game_state_t {
    syn_changelog_t changelog_;
    syn_map_t map_;
    player_manager_t player_manager_;
    ticker_t ticker_;
} game_state_t;

void game_state_init(game_state_t* game_state, const map_template_t* map_template);
void game_state_destroy(game_state_t* game_state);

#endif
