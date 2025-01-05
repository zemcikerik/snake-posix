#include "syn_changelog.h"

#include <string.h>

DEFINE_SYN_IMPLEMENTATION(changelog_t)

void syn_changelog_for_each_copy(syn_changelog_t* self, const syn_changelog_for_each_fn for_each_fn, void* ctx) {
    changelog_t* changelog = syn_changelog_t_acquire(self);

    const size_t size = changelog->size_;
    change_t* changes = malloc(size * sizeof(change_t));
    memcpy(changes, changelog->changes_, size * sizeof(change_t));

    syn_changelog_t_release(self);

    for (size_t i = 0; i < size; ++i) {
        for_each_fn(&changes[i], ctx);
    }

    free(changes);
};
