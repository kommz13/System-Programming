#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions_worker.h"
#include "extio.h"

int main_worker_from_fork(char * inputdir, int pipe_b, int in, int out) {
    int i, h1 = 1033, h2 = 1033, h3 = 1033, b = 4096;
    
    int total_countries;
    
    char serverIP[20];
    int serverPort;
    
    // receive serverIP and serverIP PORT from master
    
    readall(in, serverIP, sizeof(serverIP));
    
    readall(in, &serverPort, sizeof(serverPort));
    
    readall(in, &total_countries, sizeof(total_countries));
    
    if (total_countries >100 ) {
        printf("overflow! \n");
        exit(1);
    }
    
    if (total_countries < 0 ) {
        printf("underflow! \n");
        exit(1);
    }
    
    char ** countries = malloc(sizeof (char*)*total_countries);

    for (i=0;i<total_countries;i++) {
        countries[i] = malloc(sizeof(char)*pipe_b);
        readall(in, countries[i], pipe_b);
    }
    
    for (i = 0; i < total_countries; i++) {
        printf("%s ", countries[i]);
    }
    printf("\n");

    initialize(h1, h2, h3, 12*1024, total_countries, countries);

    createDataStructures();

    loaddir(inputdir);

    char ipbuffer[20];
    int port = createSocket(ipbuffer);
    
    printf("inputfile     = %s \n", inputdir);
    printf("h1            = %d \n", h1);
    printf("h2            = %d \n", h2);
    printf("b             = %d \n", b);
    printf("pipe b        = %d \n", pipe_b);
    printf("serverIP      = %s \n", serverIP);
    printf("serverPort    = %d \n", serverPort);
    printf("countries     = (%d) \n", total_countries);
    printf("IP            = (%s) \n", ipbuffer);
    printf("port          = (%d) \n", port);
    
    sendCountriesToServer(pipe_b, serverIP, serverPort, ipbuffer, port);
    
    
    interactWithSocket(pipe_b, in, out);
    
    cleanUpDataStructures();
    
    cleanUpSocket();

    free(countries);

    return 0;
}