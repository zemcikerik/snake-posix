#ifndef INCLUDE_LIBSNAKE_SYN_CHANGELOG_H
#define INCLUDE_LIBSNAKE_SYN_CHANGELOG_H

#include "changelog.h"
#include "../syn_template.h"

DEFINE_SYN_DECLARATION(changelog_t);

typedef void (*syn_changelog_for_each_fn)(const change_t* change, void* ctx);
change_t* syn_changelog_copy(syn_changelog_t* self, size_t* out_size);
void syn_changelog_free_copy(change_t* changes);
void syn_changelog_for_each_copy(syn_changelog_t* self, syn_changelog_for_each_fn for_each_fn, void* ctx);

#endif
