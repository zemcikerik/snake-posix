#include "changelog.h"

void changelog_init(changelog_t* self) {
    self->size_ = 0;
}

void changelog_clear(changelog_t* self) {
    self->size_ = 0;
}

void changelog_push_tile_state_change(changelog_t* self, const coordinates_t coordinates, const map_tile_t map_tile) {
    change_t* change = &self->changes_[self->size_++];
    change->type_ = CHANGE_TILE_STATE;
    change->data_.change_tile_state_data_.coordinates_ = coordinates;
    change->data_.change_tile_state_data_.tile_data_ = map_tile;
}
