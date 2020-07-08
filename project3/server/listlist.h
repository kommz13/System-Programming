#ifndef LISTLIST_H
#define LISTLIST_H

#include "CountryData.h"


typedef struct InnerNode {
    struct CountryData * countryData;
    struct InnerNode * next;
} InnerNode;

typedef struct OuterNode {
    char ip[20];    
    int port; // hash value
    int socket_b; // hash value
    struct InnerNode * up;
    struct OuterNode * next;
} OuterNode;


void initializeLL();

void cleanupLL();

OuterNode * insertLL_outer(char * ip, int port, int socket_b);

OuterNode * searchLL(char * ip, int port);

void insertLL(char * ip, int port, int socket_b, CountryData * cdata);

void printLL();

int getSocketB();

int numWorkers();

void connectToAll(int * fds);

void connectSingle(int * fds, char * country);

#endif


