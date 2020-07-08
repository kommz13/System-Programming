#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#include "queue.h"


void initializeQ(Queue * queue, int queue_b) {
    queue->start = 0;
    queue->end = -1;
    queue->count = 0;
    queue->data = malloc(sizeof(int)*queue_b);
    queue->queue_b = queue_b;
    
    pthread_mutex_init(&queue->mtx, 0);
    pthread_cond_init(&queue->cond_nonempty, 0);
    pthread_cond_init(&queue->cond_nonfull, 0);
}

void cleanupQ(Queue * queue) {
    pthread_cond_destroy(&queue->cond_nonempty);
    pthread_cond_destroy(&queue->cond_nonfull);
    pthread_mutex_destroy(&queue->mtx);
}

void placeQ(Queue * queue, int data) {
    int POOL_SIZE= queue->queue_b;
    pthread_mutex_lock(&queue->mtx);
    while (queue->count >= POOL_SIZE) {
        pthread_cond_wait(&queue->cond_nonfull, &queue->mtx);
    }
    queue->end = (queue->end + 1) % POOL_SIZE;
    queue->data[queue->end] = data;
    queue->count++;
    pthread_mutex_unlock(&queue->mtx);
}

int obtainQ(Queue * queue) {
    int POOL_SIZE= queue->queue_b;
    int data = 0;
    pthread_mutex_lock(&queue->mtx);
    while (queue->count <= 0) {
        pthread_cond_wait(&queue->cond_nonempty, &queue->mtx);
    }
    data = queue->data[queue->start];
    queue->start = (queue->start + 1) % POOL_SIZE;
    queue->count--;
    pthread_mutex_unlock(&queue->mtx);
    return data;
}

void notifyConsumers(Queue * queue) {
    pthread_cond_broadcast(&queue->cond_nonempty);
}
    
void notifyProducer(Queue * queue) {
    pthread_cond_broadcast(&queue->cond_nonfull);
}

