#ifndef INCLUDE_LIBSNAKE_SYN_TEMPLATE_H
#define INCLUDE_LIBSNAKE_SYN_TEMPLATE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "syn_helpers.h"

#define DEFINE_SYN_DECLARATION(T)            \
    typedef struct syn_##T {                 \
        T obj_;                              \
        pthread_mutex_t mutex_;              \
    } syn_##T;                               \
                                             \
    void syn_##T##_init(syn_##T* self);      \
    void syn_##T##_destroy(syn_##T* self);   \
                                             \
    T* syn_##T##_acquire(syn_##T* self);     \
    T* syn_##T##_try_acquire(syn_##T* self); \
    void syn_##T##_release(syn_##T* self);   \
    T* syn_##T##_peek(syn_##T* self);

#define DEFINE_SYN_IMPLEMENTATION(T) \
    void syn_##T##_init(syn_##T* self) {    \
        init_pthread_shared_mutex(&self->mutex_);   \
    }   \
    \
    void syn_##T##_destroy(syn_##T* self) {   \
        if (pthread_mutex_destroy(&self->mutex_)) {   \
            perror("syn_" #T "_destroy pthread_mutex_destroy() error");   \
            exit(EXIT_FAILURE);   \
        }   \
    }   \
    \
    T* syn_##T##_acquire(syn_##T* self) {   \
        pthread_mutex_lock(&self->mutex_);   \
        return &self->obj_;   \
    }   \
    \
    T* syn_##T##_try_acquire(syn_##T* self) {   \
        return pthread_mutex_trylock(&self->mutex_) ? NULL : &self->obj_;   \
    }   \
    \
    void syn_##T##_release(syn_##T* self) {   \
        pthread_mutex_unlock(&self->mutex_);   \
    }   \
    \
    T* syn_##T##_peek(syn_##T* self) {   \
        return &self->obj_;   \
    }

#endif
