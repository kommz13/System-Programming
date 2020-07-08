#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>      /* sockets */
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <pthread.h>
#include <netdb.h>
#include "extio.h"
#include "Request.h"
#include "functions_client.h"

static int common_counter = 0;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t lockscreen = PTHREAD_MUTEX_INITIALIZER;

int main_worker_from_pthread_create(int *id, int total_commands, char **commands, char * serverIp, int serverPort, int workers) {
    int i;
    int _id = *id;
    char buffer[5000];

    pthread_mutex_lock(&lock);
    common_counter++;

    printf("Thread ready: %d of %d \n", common_counter, workers);

    if (common_counter == workers) {
        printf("success ! \n");
    }
    pthread_cond_broadcast(&cond);

    while (common_counter < workers) {
        pthread_cond_wait(&cond, &lock);
    }

    pthread_mutex_unlock(&lock);


    struct sockaddr_in server = {0};
    struct sockaddr *serverptr = (struct sockaddr*) &server;

    server.sin_family = AF_INET; /* Internet domain */

    struct hostent rem = {0};

    for (i = 0; i < total_commands; i++) {
        if (i % workers == *id) {
            printf("Worker %d: processing: %s \n", *id, commands[i]);

            int newsockfd;

            if ((newsockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                exit(1);
            }

            pthread_mutex_lock(&lock);
            memcpy(&rem, gethostbyname(serverIp), sizeof (struct hostent));
            pthread_mutex_unlock(&lock);

            memcpy(&server.sin_addr, rem.h_addr, rem.h_length);
            server.sin_port = htons(serverPort); /* Server port */

            if (connect(newsockfd, serverptr, sizeof (server)) < 0) {
                perror("ERROR: connect");
                break;
            }

            // TODO: send status  to server  (2=client)
            Response response = {0};

            response.status = 2; // I am a worker
            response.number = 0;

            writeall(newsockfd, &response, sizeof (Response));

            // receive socket B

            readall(newsockfd, &response, sizeof (Response));

            printf("Client %d: received: %d %d \n", _id, response.number, response.status);

            int socket_b = response.number;

            //            printf("### SocketB: %d \n", socket_b);


            // send command:
            strcpy(buffer, commands[i]);

            int length = strlen(buffer);

            if (length < 3) {
                printf("unknown first token \n");
                continue;
            }

            char * ptr = NULL;
            char * firsttoken = strtok_r(buffer, " ", &ptr);

            if (strcmp(firsttoken, "searchPatientRecord") == 0 || strcmp(firsttoken, "sp") == 0) {
                char * secondtoken = strtok_r(NULL, " ", &ptr);

                if (secondtoken == NULL) {
                    printf("second token invalid for searchPatientRecord \n");
                } else {
                    char * tokenA = secondtoken;

                    if (tokenA != NULL) {
                        requestPatientRecord(commands[i], &lockscreen, newsockfd, socket_b, tokenA);
                    }
                }
            } else if (strcmp(firsttoken, "diseaseFrequency") == 0 || strcmp(firsttoken, "dfreq") == 0) {
                char * diseasetoken = strtok_r(NULL, " ", &ptr);
                char * secondtoken = strtok_r(NULL, " ", &ptr);
                char * thirdtoken = strtok_r(NULL, " ", &ptr);
                char * countrytoken = strtok_r(NULL, " ", &ptr);

                if (diseasetoken == NULL) {
                    printf("invalid disease \n");
                } else if (secondtoken == NULL) {
                    printf("Invalid date \n");
                } else {
                    char * dateA = secondtoken;
                    if (dateA != NULL) {

                        char * dateB = thirdtoken;
                        if (dateB == NULL) {
                            printf("third token invalid \n");
                        } else if (countrytoken == NULL) {
                            requestDiseaseFrequencyWithoutCountry(commands[i], &lockscreen, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken);
                        } else {
                            requestDiseaseFrequencyWithCountry(commands[i], &lockscreen, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken, countrytoken);
                        }
                    }
                }
            } else if (strcmp(firsttoken, "topk-AgeRanges") == 0 || strcmp(firsttoken, "tar") == 0) {
                char * ktoken = strtok_r(NULL, " ", &ptr);
                char * countrytoken = strtok_r(NULL, " ", &ptr);
                char * diseasetoken = strtok_r(NULL, " ", &ptr);
                char * secondtoken = strtok_r(NULL, " ", &ptr);
                char * thirdtoken = strtok_r(NULL, " ", &ptr);

                if (ktoken == NULL) {
                    printf("invalid k \n");
                } else if (countrytoken == NULL) {
                    printf("invalid countrytoken \n");
                } else if (diseasetoken == NULL) {
                    printf("invalid disease \n");
                } else if (secondtoken == NULL) {
                    printf("Invalid date \n");
                } else if (thirdtoken == NULL) {
                    printf("Invalid date \n");
                } else {
                    topKAgeRanges(commands[i], &lockscreen, newsockfd, socket_b, ktoken, countrytoken, diseasetoken, secondtoken, thirdtoken);
                }
            } else if (strcmp(firsttoken, "numPatientAdmissions") == 0 || strcmp(firsttoken, "npa") == 0) {
                char * diseasetoken = strtok_r(NULL, " ", &ptr);
                char * secondtoken = strtok_r(NULL, " ", &ptr);
                char * thirdtoken = strtok_r(NULL, " ", &ptr);
                char * countrytoken = strtok_r(NULL, " ", &ptr);

                if (diseasetoken == NULL) {
                    printf("invalid disease \n");
                } else if (secondtoken == NULL) {
                    printf("Invalid date \n");
                } else {
                    char * dateA = secondtoken;
                    if (dateA != NULL) {

                        char * dateB = thirdtoken;
                        if (dateB == NULL) {
                            printf("third token invalid \n");
                        } else if (countrytoken == NULL) {
                            patientAdmissionsWithoutCountry(commands[i], &lockscreen, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken);
                        } else {
                            patientAdmissionsWithCountry(commands[i], &lockscreen, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken, countrytoken);
                        }
                    }
                }
            } else if (strcmp(firsttoken, "numPatientDischarges") == 0 || strcmp(firsttoken, "npd") == 0) {
                char * diseasetoken = strtok_r(NULL, " ", &ptr);
                char * secondtoken = strtok_r(NULL, " ", &ptr);
                char * thirdtoken = strtok_r(NULL, " ", &ptr);
                char * countrytoken = strtok_r(NULL, " ", &ptr);

                if (diseasetoken == NULL) {
                    printf("invalid disease \n");
                } else if (secondtoken == NULL) {
                    printf("Invalid date \n");
                } else {
                    char * dateA = secondtoken;
                    if (dateA != NULL) {

                        char * dateB = thirdtoken;
                        if (dateB == NULL) {
                            printf("third token invalid \n");
                        } else if (countrytoken == NULL) {
                            patientDischargesWithoutCountry(commands[i], &lockscreen, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken);
                        } else {
                            patientDischargesWithCountry(commands[i], &lockscreen, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken, countrytoken);
                        }
                    }
                }
            }


            close(newsockfd);
        }
    }

    return 0;
}
