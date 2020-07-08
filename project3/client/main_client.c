#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "functions_client.h"
#include "Request.h"

int main(int argc, char** argv) {
    char * queryfile = NULL;
    int i, w;
    char ** commands = NULL;
    int total_commands = 0;
    char * serverIp = NULL;
    int serverPort = 0; 
    
    for (i = 1; i < argc; i = i + 2) {
        if (!strcmp(argv[i], "-q")) {
            queryfile = argv[i + 1];
        }
        if (!strcmp(argv[i], "-w")) {
            w = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-sip")) {
            serverIp = argv[i + 1];
        }
        if (!strcmp(argv[i], "-sp")) {
            serverPort = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-w")) {
            w = atoi(argv[i + 1]);
        }
        
    }

    printf("queryfile          = %s \n", queryfile);
    printf("w                  = %d \n", w);
    printf("sip                = %s \n", serverIp);
    printf("sp                 = %d \n", serverPort);
    
    if (serverPort < 0 || w < 0) {
        printf("pipe_b should be greater than %lu and %lu \n", sizeof(Response), sizeof(Request));
        return -1;
    }
    
    loadfileClient(queryfile, &total_commands, &commands);
    
    initializeClient(w, total_commands, commands, serverIp, serverPort);

    cleanUpDataStructuresClient(total_commands, commands);

    interactWithUserClient();

    return 0;
}
