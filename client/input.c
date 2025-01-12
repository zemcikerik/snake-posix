#include "input.h"
#include <stdbool.h>
#include <termbox2.h>
#include "../libsnake/constants.h"

void noop(const int) {
}

void input_init_sig() {
    struct sigaction destroyed_sigaction;
    destroyed_sigaction.sa_handler = noop;
    destroyed_sigaction.sa_flags = 0;
    sigemptyset(&destroyed_sigaction.sa_mask);
    sigaction(SIG_DESTROYED, &destroyed_sigaction, NULL);
}

void* input_main(void* arg) {
    input_t* self = arg;

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIG_DESTROYED);
    pthread_sigmask(SIG_SETMASK, &set, NULL);

    while (!self->shutdown_) {
        struct tb_event event;

        if (tb_poll_event(&event) != TB_OK || self->shutdown_) {
            break;
        }

        if (event.type == TB_EVENT_RESIZE) {
            self->callbacks_.on_resize(self->callbacks_.ctx_);
            continue;
        }

        if (event.type != TB_EVENT_KEY) {
            continue;
        }

        if (event.key == TB_KEY_ARROW_UP || event.key == TB_KEY_ARROW_DOWN
            || event.key == TB_KEY_ARROW_LEFT || event.key == TB_KEY_ARROW_RIGHT) {
            self->callbacks_.on_arrow_press(event.key, self->callbacks_.ctx_);
        } else if (event.key == TB_KEY_ESC) {
            self->callbacks_.on_exit_press(self->callbacks_.ctx_);
        } else if (event.ch == 'p') {
            self->callbacks_.on_pause_press(self->callbacks_.ctx_);
        } else if (event.ch == 'u') {
            self->callbacks_.on_unpause_press(self->callbacks_.ctx_);
        } else if (event.ch == 'r') {
            self->callbacks_.on_respawn_press(self->callbacks_.ctx_);
        }
    }

    return NULL;
}

void input_init(input_t* self, input_callbacks_t* callbacks) {
    self->callbacks_ = *callbacks;
    self->shutdown_ = false;
    pthread_create(&self->thread_, NULL, input_main, self);
}

void input_destroy(input_t* self) {
    self->shutdown_ = true;
    pthread_kill(self->thread_, SIG_DESTROYED);
    pthread_join(self->thread_, NULL);
}
