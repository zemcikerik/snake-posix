#include "syn_changelog.h"
#include <string.h>

DEFINE_SYN_IMPLEMENTATION(changelog_t)

change_t* syn_changelog_copy(syn_changelog_t* self, size_t* out_size) {
    const changelog_t* changelog = syn_changelog_t_acquire(self);
    const size_t size = changelog->size_;

    if (size == 0) {
        syn_changelog_t_release(self);
        *out_size = 0;
        return NULL;
    }

    change_t* changes = malloc(size * sizeof(change_t));
    memcpy(changes, changelog->changes_, size * sizeof(change_t));

    syn_changelog_t_release(self);
    *out_size = size;
    return changes;
}

void syn_changelog_free_copy(change_t* changes) {
    free(changes);
}

void syn_changelog_for_each_copy(syn_changelog_t* self, const syn_changelog_for_each_fn for_each_fn, void* ctx) {
    size_t size;
    change_t* changes = syn_changelog_copy(self, &size);

    if (changes == NULL) {
        return;
    }

    for (size_t i = 0; i < size; ++i) {
        for_each_fn(&changes[i], ctx);
    }

    syn_changelog_free_copy(changes);
};
