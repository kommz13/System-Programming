#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>      /* sockets */
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <unistd.h>
#include <netdb.h>          /* read, write, close */

#include "listlist.h"

struct OuterNode * _head = NULL;
pthread_mutex_t mutexLL;
int outerNodeCounter = 0;

void initializeLL() {
    pthread_mutex_init(&mutexLL, NULL);
}

void cleanupLL() {
    OuterNode * outerPointer = _head;

    if (outerPointer == NULL) {
        return;
    } else {
        while (outerPointer != NULL) {
            OuterNode * temp = outerPointer;
            outerPointer = outerPointer->next;

            InnerNode* innerPointer = temp->up;

            if (innerPointer != NULL) {
                while (innerPointer != NULL) {
                    InnerNode * temp2 = innerPointer;
                    innerPointer = innerPointer->next;

                    free(temp2);
                }
            }

            free(temp);
        }
    }

    pthread_mutex_destroy(&mutexLL);
}

InnerNode * create_innernode(CountryData * country) {
    InnerNode* newnode = (InnerNode *) malloc(sizeof (InnerNode));
    newnode->countryData = country;
    newnode->next = NULL;
    return newnode;
}

OuterNode * create_outernode(char * ip, int port, int socket_b) {
    OuterNode* newnode = (OuterNode *) malloc(sizeof (OuterNode));
    newnode->up = NULL;
    newnode->next = NULL;

    strcpy(newnode->ip, ip);
    newnode->port = port;
    newnode->socket_b = socket_b;

    outerNodeCounter++;

    return newnode;
}

void insertLL(char * ip, int port, int socket_b, CountryData * cdata) {
    pthread_mutex_lock(&mutexLL);
    OuterNode * horizontalPointer = _head;

    if (horizontalPointer == NULL) {
        horizontalPointer = create_outernode(ip, port, socket_b);
        _head = horizontalPointer;
    } else {
        while (horizontalPointer != NULL) {
            if (horizontalPointer->port == port) {
                break;
            } else {
                if (horizontalPointer->next) {
                    horizontalPointer = horizontalPointer->next;
                } else {
                    horizontalPointer->next = create_outernode(ip, port, socket_b);
                    horizontalPointer = horizontalPointer->next;
                    break;
                }
            }
        }
    }

    InnerNode* verticalNode = horizontalPointer->up;

    if (verticalNode == NULL) {
        horizontalPointer->up = create_innernode(cdata);
    } else {
        InnerNode* n = create_innernode(cdata);
        n->next = horizontalPointer->up;
        horizontalPointer->up = n;
    }
    pthread_mutex_unlock(&mutexLL);
}

OuterNode * searchLL(char * ip, int port) {
    pthread_mutex_lock(&mutexLL);
    OuterNode * horizontalPointer = _head;

    if (horizontalPointer != NULL) {
        while (horizontalPointer != NULL) {
            if (strcmp(horizontalPointer->ip, ip) == 0) {
                if (horizontalPointer->port == port) {
                    pthread_mutex_unlock(&mutexLL);
                    return horizontalPointer;
                }
            }
            horizontalPointer = horizontalPointer->next;
        }
    }

    pthread_mutex_unlock(&mutexLL);
    return NULL;
}

void printLL() {
    pthread_mutex_lock(&mutexLL);

    OuterNode * outerPointer = _head;

    if (outerPointer == NULL) {
        pthread_mutex_unlock(&mutexLL);
        return;
    } else {
        while (outerPointer != NULL) {
            printf("Node: %s:%d: ", outerPointer->ip, outerPointer->port);

            InnerNode* innerPointer = outerPointer->up;

            if (innerPointer != NULL) {
                while (innerPointer != NULL) {
                    printf("\t '%s' \n", innerPointer->countryData->name);
                    innerPointer = innerPointer->next;
                }
            }

            outerPointer = outerPointer->next;
        }
    }

    pthread_mutex_unlock(&mutexLL);
}

int getSocketB() {
    pthread_mutex_lock(&mutexLL);
    OuterNode * outerPointer = _head;
    int socket_b;
    if (outerPointer != NULL) {
        socket_b = outerPointer->socket_b;
    } else {
        printf("UNKNOWN SOCKET B \n");
        exit(1);
    }
    pthread_mutex_unlock(&mutexLL);
    return socket_b;

}

int numWorkers() {
    pthread_mutex_lock(&mutexLL);
    int n = outerNodeCounter;
    pthread_mutex_unlock(&mutexLL);
    return n;
}

void connectToAll(int * fds) {
    pthread_mutex_lock(&mutexLL);

    OuterNode * outerPointer = _head;

    if (outerPointer == NULL) {
        pthread_mutex_unlock(&mutexLL);
        return;
    } else {
        int i = 0;
        while (outerPointer != NULL) {
            printf("Connecting to node: %s:%d \n", outerPointer->ip, outerPointer->port);


            int sock;
            struct sockaddr_in server;
            struct sockaddr *serverptr = (struct sockaddr*) &server;
            struct hostent *rem;

            if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                exit(1);
            }

            if ((rem = gethostbyname(outerPointer->ip)) == NULL) {
                herror("gethostbyname");
                exit(1);
            }

            server.sin_family = AF_INET; /* Internet domain */
            memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
            server.sin_port = htons(outerPointer->port); /* Server port */

            if (connect(sock, serverptr, sizeof (server)) < 0) {
                perror("connect");
                exit(1);
            }

            fds[i] = sock;

            i++;

            outerPointer = outerPointer->next;
        }
    }


    pthread_mutex_unlock(&mutexLL);
}

void connectSingle(int * fd, char * country) {
    pthread_mutex_lock(&mutexLL);

    OuterNode * outerPointer = _head;

    fd[0] = -1;

    if (outerPointer == NULL) {
        pthread_mutex_unlock(&mutexLL);
        return;
    } else {
        while (outerPointer != NULL) {
            printf("Checking ... node: %s:%d \n", outerPointer->ip, outerPointer->port);

            InnerNode* innerPointer = outerPointer->up;

            if (innerPointer != NULL) {
                while (innerPointer != NULL) {
                    if (strcmp(innerPointer->countryData->name, country) == 0) {

                        printf("Connecting to node: %s:%d \n", outerPointer->ip, outerPointer->port);

                        int sock;
                        struct sockaddr_in server;
                        struct sockaddr *serverptr = (struct sockaddr*) &server;
                        struct hostent rem = {0};

                        if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                            perror("socket");
                            exit(1);
                        }

                        memcpy(&rem, gethostbyname(outerPointer->ip), sizeof (struct hostent));

                        server.sin_family = AF_INET; /* Internet domain */
                        memcpy(&server.sin_addr, rem.h_addr, rem.h_length);
                        server.sin_port = htons(outerPointer->port); /* Server port */

                        if (connect(sock, serverptr, sizeof (server)) < 0) {
                            perror("connect");
                            exit(1);
                        }

                        fd[0] = sock;

                         pthread_mutex_unlock(&mutexLL);
                         
                        return;
                    }
                    innerPointer = innerPointer->next;
                }
            }

            outerPointer = outerPointer->next;
        }
    }


    pthread_mutex_unlock(&mutexLL);
}

