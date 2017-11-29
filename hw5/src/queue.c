#include "queue.h"
#include "pthread.h"
#include <errno.h>

/*
typedef struct queue_node_t {
    void *item;
    struct queue_node_t *next;
} queue_node_t;

typedef struct queue_t {
    queue_node_t *front, *rear;
    sem_t items;
    pthread_mutex_t lock;
    bool invalid;
} queue_t;
*/

queue_t *create_queue(void) {
    queue_t *new_q;
    if((new_q = (queue_t*)calloc(1,sizeof(queue_t))) == NULL){
        return NULL;
    }
    if(sem_init(&new_q->items,0,0) != 0){
        return NULL;
    }
    if(pthread_mutex_init(&new_q->lock,NULL) != 0){
        return NULL;
    }
    return new_q;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    if(self == NULL || destroy_function == NULL){
        errno = EINVAL;
        return false;
    }
    queue_node_t *curr_node = self->front;
    queue_node_t *prev_node;
    pthread_mutex_lock(&self->lock);
    if(!self->invalid){
        int size = 0;
        for(int i = 0; i < sem_getvalue(&self->items,&size); i++){
            destroy_function(curr_node);
            prev_node = curr_node;
            curr_node = curr_node->next;
            free(prev_node);
        }
        self->invalid = true;
        pthread_mutex_unlock(&self->lock);
        return true;
    }
    pthread_mutex_unlock(&self->lock);
    errno = EINVAL;
    return false;
}

bool enqueue(queue_t *self, void *item) {
    if(self == NULL || item == NULL){
        errno = EINVAL;
        return false;
    }
    queue_node_t *new_node;
    if((new_node = (queue_node_t*)calloc(1,sizeof(queue_node_t))) == NULL){
        return false;
    }
    new_node->item = item;
    new_node->next = NULL;
    pthread_mutex_lock(&self->lock);
    if(self->invalid){
        free(new_node);
        errno = EINVAL;
        pthread_mutex_unlock(&self->lock);
        return false;
    }
    int size;
    sem_getvalue(&self->items,&size);
    if(size == 0){
        self->rear = new_node;
        self->front = new_node;
    }
    else{
        self->rear->next = new_node;
        self->rear = new_node;
    }
    sem_post(&self->items);
    pthread_mutex_unlock(&self->lock);
    return true;
}

void *dequeue(queue_t *self) {
    if(!self){
        errno = EINVAL;
        return false;
    }
    if(self->front == NULL){
        return NULL;
    }
    queue_node_t *prev_node;
    pthread_mutex_lock(&self->lock);
    if(self->invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&self->lock);
        return false;
    }
    int size;
    void* item = self->front->item;
    sem_getvalue(&self->items,&size);
    if(size > 0){
        prev_node = self->front;
        self->front = self->front->next;
        free(prev_node);
        sem_wait(&self->items);
        if(size == 1){
            self->rear = self->front;
        }
    }
    pthread_mutex_unlock(&self->lock);
    return item;
}
