#ifndef INCLUDE_LIBSNAKE_SYN_HELPERS_H
#define INCLUDE_LIBSNAKE_SYN_HELPERS_H

#include <pthread.h>

void init_pthread_shared_mutex(pthread_mutex_t* mutex);
void init_pthread_shared_cond(pthread_cond_t* cond);

#endif
