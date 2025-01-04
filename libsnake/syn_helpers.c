#include "syn_helpers.h"
#include <stdlib.h>
#include <stdio.h>

void init_pthread_shared_mutex(pthread_mutex_t* mutex) {
    pthread_mutexattr_t mutex_attr;

    if (pthread_mutexattr_init(&mutex_attr)) {
        perror("pthread_mutexattr_init() error");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED)) {
        perror("pthread_mutexattr_setpshared() error");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(mutex, &mutex_attr)) {
        perror("pthread_mutex_init() error");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_destroy(&mutex_attr)) {
        perror("pthread_mutexattr_destroy() error");
        exit(EXIT_FAILURE);
    }
}

void init_pthread_shared_cond(pthread_cond_t* cond) {
    pthread_condattr_t cond_attr;

    if (pthread_condattr_init(&cond_attr)) {
        perror("pthread_condattr_init() error");
        exit(EXIT_FAILURE);
    }

    if (pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED)) {
        perror("pthread_condattr_setpshared() error");
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(cond, &cond_attr)) {
        perror("pthread_cond_init() error");
        exit(EXIT_FAILURE);
    }

    if (pthread_condattr_destroy(&cond_attr)) {
        perror("pthread_condattr_destroy() error");
        exit(EXIT_FAILURE);
    }
}
