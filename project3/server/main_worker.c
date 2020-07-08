#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions_worker.h"
#include "extio.h"



int main_worker_from_pthread_create(int id, int workers, Queue * queue) {
     printf("Thread ready: %d of %d \n", id+1, workers);
     
    initialize(id, queue);

    interactWithQueue();
        
    return 0;
}