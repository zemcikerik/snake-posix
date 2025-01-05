#ifndef INCLUDE_LIBSNAKE_TICKER_H
#define INCLUDE_LIBSNAKE_TICKER_H

#include <pthread.h>
#include <stddef.h>

typedef struct ticker_t {
    size_t tick_;
    pthread_mutex_t tick_mutex_;
    pthread_cond_t next_tick_cond_;
} ticker_t;

typedef struct ticker_observer_t {
    ticker_t* ticker_;
    size_t last_tick_;
} ticker_observer_t;

void ticker_init(ticker_t* self);
void ticker_destroy(ticker_t* self);
void ticker_tick(ticker_t* self);

void ticker_observer_init(ticker_observer_t* self, ticker_t* ticker);
size_t ticker_observer_wait_for_next_tick(ticker_observer_t* self);

#endif
