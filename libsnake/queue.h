#ifndef INCLUDE_CLIENT_QUEUE_H
#define INCLUDE_CLIENT_QUEUE_H

#include <stdbool.h>

#define DEFINE_QUEUE_DECLARATION(T)   \
    typedef struct queue_##T##_node_t {   \
        T item_;   \
        struct queue_##T##_node_t* next_;   \
    } queue_##T##_node_t;   \
    \
    typedef struct queue_##T {   \
        queue_##T##_node_t* head_;   \
        queue_##T##_node_t* tail_;   \
    } queue_##T;   \
    \
    void queue_##T##_init(queue_##T* self);   \
    void queue_##T##_destroy(queue_##T* self);   \
    \
    void queue_##T##_push(queue_##T* self, const T* item);   \
    bool queue_##T##_pop(queue_##T* self, T* out_item);   \
    void queue_##T##_clear(queue_##T* self);   \
    \
    bool queue_##T##_is_empty(const queue_##T* self);

#define DEFINE_QUEUE_IMPLEMENTATION(T)   \
    void queue_##T##_init(queue_##T* self) {   \
        self->head_ = NULL;   \
        self->tail_ = NULL;   \
    }   \
    \
    void queue_##T##_destroy(queue_##T* self) {   \
        queue_##T##_clear(self);   \
    }   \
    \
    void queue_##T##_push(queue_##T* self, const T* item) {   \
        queue_##T##_node_t* node = malloc(sizeof(queue_##T##_node_t));   \
        node->item_ = *item;   \
        node->next_ = NULL;   \
        \
        if (self->head_ != NULL) {   \
            self->tail_->next_ = node;   \
            self->tail_ = node;   \
        } else {   \
            self->head_ = node;   \
            self->tail_ = node;   \
        }   \
    }   \
    \
    bool queue_##T##_pop(queue_##T* self, T* out_item) {    \
        if (queue_##T##_is_empty(self)) {   \
            return false;   \
        }   \
        *out_item = self->head_->item_;   \
        queue_##T##_node_t* next = self->head_->next_;   \
        free(self->head_);   \
        self->head_ = next;   \
        return true;   \
    }   \
    \
    void queue_##T##_clear(queue_##T* self) {   \
        for (queue_##T##_node_t* node = self->head_; node != NULL;) {   \
            queue_##T##_node_t* next = node->next_;   \
            free(node);   \
            node = next;   \
        }   \
        \
        self->head_ = NULL;   \
        self->tail_ = NULL;   \
    }   \
    \
    bool queue_##T##_is_empty(const queue_##T* self) {   \
        return self->head_ == NULL;   \
    }

#endif
