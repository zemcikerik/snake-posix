#ifndef INCLUDE_CLIENT_INPUT_H
#define INCLUDE_CLIENT_INPUT_H

#include <stdatomic.h>
#include <stdint.h>
#include <pthread.h>

typedef struct input_callbacks_t {
    void (*on_arrow_press)(uint16_t arrow, void* ctx);
    void (*on_exit_press)(void* ctx);
    void (*on_pause_press)(void* ctx);
    void (*on_unpause_press)(void* ctx);
    void (*on_respawn_press)(void* ctx);
    void (*on_resize)(void* ctx);
    void* ctx_;
} input_callbacks_t;

typedef struct input_t {
    pthread_t thread_;
    input_callbacks_t callbacks_;
    atomic_bool shutdown_;
} input_t;

void input_init_sig();
void input_init(input_t* self, input_callbacks_t* callbacks);
void input_destroy(input_t* self);

#endif
