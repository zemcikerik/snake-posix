#include "game_state.h"

void game_state_init(game_state_t* game_state, const map_template_t* map_template) {
    syn_changelog_t_init(&game_state->changelog_);
    changelog_init(syn_changelog_t_peek(&game_state->changelog_));

    syn_map_t_init(&game_state->map_);
    map_init_from_template(syn_map_t_peek(&game_state->map_), map_template);

    player_manager_init(&game_state->player_manager_);
    ticker_init(&game_state->ticker_);
}

void game_state_destroy(game_state_t* game_state) {
    syn_changelog_t_destroy(&game_state->changelog_);
    syn_map_t_destroy(&game_state->map_);
    player_manager_destroy(&game_state->player_manager_);
    ticker_destroy(&game_state->ticker_);
}
