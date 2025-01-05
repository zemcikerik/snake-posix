#include "ticker.h"
#include "syn_helpers.h"

void ticker_init(ticker_t* self) {
    self->tick_ = 0;
    init_pthread_shared_mutex(&self->tick_mutex_);
    init_pthread_shared_cond(&self->next_tick_cond_);
}

void ticker_destroy(ticker_t* self) {
    pthread_mutex_destroy(&self->tick_mutex_);
    pthread_cond_destroy(&self->next_tick_cond_);
}

void ticker_tick(ticker_t* self) {
    pthread_mutex_lock(&self->tick_mutex_);
    self->tick_ += 1;
    pthread_cond_broadcast(&self->next_tick_cond_);
    pthread_mutex_unlock(&self->tick_mutex_);
}

void ticker_observer_init(ticker_observer_t* self, ticker_t* ticker) {
    self->ticker_ = ticker;

    pthread_mutex_lock(&ticker->tick_mutex_);
    self->last_tick_ = ticker->tick_;
    pthread_mutex_unlock(&ticker->tick_mutex_);
}

size_t ticker_observer_wait_for_next_tick(ticker_observer_t* self) {
    pthread_mutex_lock(&self->ticker_->tick_mutex_);

    while (self->last_tick_ == self->ticker_->tick_) {
        pthread_cond_wait(&self->ticker_->next_tick_cond_, &self->ticker_->tick_mutex_);
    }

    const size_t difference = self->ticker_->tick_ - self->last_tick_;
    self->last_tick_ = self->ticker_->tick_;
    pthread_mutex_unlock(&self->ticker_->tick_mutex_);

    return difference;
}
