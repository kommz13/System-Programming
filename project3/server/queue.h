
#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
    int * data;
    int start;
    int end;
    int count;
    int queue_b;

    pthread_mutex_t mtx;
    pthread_cond_t cond_nonempty, cond_nonfull;
} Queue;

void placeQ(Queue * queue, int data);

int obtainQ(Queue * queue);

void notifyConsumers(Queue * queue);

void notifyProducer(Queue * queue);

void initializeQ(Queue * queue, int queue_b);

void cleanupQ(Queue * queue);

#endif /* QUEUE_H */

