#include "game_state.h"

void game_state_init(game_state_t* self, const map_template_t* map_template) {
    syn_changelog_t_init(&self->changelog_);
    changelog_init(syn_changelog_t_peek(&self->changelog_));

    syn_map_t_init(&self->map_);
    map_init_from_template(syn_map_t_peek(&self->map_), map_template);

    player_manager_init(&self->player_manager_);
    ticker_init(&self->ticker_);
    self->game_ended_ = false;
}

void game_state_destroy(game_state_t* self) {
    syn_changelog_t_destroy(&self->changelog_);
    syn_map_t_destroy(&self->map_);
    player_manager_destroy(&self->player_manager_);
    ticker_destroy(&self->ticker_);
}
