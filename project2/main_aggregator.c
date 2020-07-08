#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "functions_worker.h"
#include "functions_aggregator.h"
#include "Request.h"

int main(int argc, char** argv) {
    char * inputdir = NULL;
    int i, w, pipe_b;
    char ** countries;
    int total_countries;

    for (i = 1; i < argc; i = i + 2) {
        if (!strcmp(argv[i], "-i")) {
            inputdir = argv[i + 1];
        }
        if (!strcmp(argv[i], "-b")) {
            pipe_b = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-w")) {
            w = atoi(argv[i + 1]);
        }
    }

    printf("inputfile          = %s \n", inputdir);
    printf("w                  = %d \n", w);
    printf("pipe_b             = %d \n", pipe_b);
    
    if (pipe_b < sizeof(Response) || pipe_b < sizeof(Request)) {
        printf("pipe_b should be greater than %lu and %lu \n", sizeof(Response), sizeof(Request));
        return -1;
    }
    
    loaddirAggregator(inputdir, &total_countries, &countries);
    
    initializeAggregator(w, pipe_b, total_countries, countries);

    createDataStructuresAggregator();

    interactWithUserAggregator();

    cleanUpDataStructuresAggregator();

    return 0;
}
