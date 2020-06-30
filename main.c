#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

int main(int argc, char** argv) {
    char * inputfile = NULL;
    int i, h1, h2, b;

    for (i = 1; i < argc; i = i + 2) {
        if (!strcmp(argv[i], "-p")) {
            inputfile = argv[i + 1];
        }
        if (!strcmp(argv[i], "-h1")) {
            h1 = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-h2")) {
            h2 = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-b")) {
            b = atoi(argv[i + 1]);
        }

    }

    //printf("inputfile     = %s \n", inputfile);
    //printf("h1            = %d \n", h1);
    //printf("h2            = %d \n", h2);
    //printf("b             = %d \n", b);

    initialize(h1, h2, 100, b);

    createDataStructures();

    loadfile(inputfile);

    interactWithUser();

    cleanUpDataStructures();

    return 0;
}

