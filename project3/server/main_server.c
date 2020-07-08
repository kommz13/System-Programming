#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "functions_worker.h"
#include "functions_server.h"
#include "Request.h"

int main(int argc, char** argv) {
    int i, w, queue_b, q, s;

    for (i = 1; i < argc; i = i + 2) {
        if (!strcmp(argv[i], "-q")) {
            q = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-s")) {
            s = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-b")) {
            queue_b = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-w")) {
            w = atoi(argv[i + 1]);
        }
    }

    printf("w                  = %d \n", w);
    printf("q                  = %d \n", q);
    printf("s                  = %d \n", s);
    printf("queue_b            = %d \n", queue_b);

    if (queue_b < 0 || q < 0 || w < 0) {
        printf("pipe_b should be greater than %lu and %lu \n", sizeof (Response), sizeof (Request));
        return -1;
    }

    initializeServer(q, s, queue_b, w, -1, 0, 0);

    createDataStructuresServer();
    
    createThreadPool();

    interactWithClientsAndWorkers();
    
    cleanUpThreadPool();

    cleanUpDataStructuresServer();

    return 0;
}
