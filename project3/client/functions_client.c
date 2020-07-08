#include  <poll.h>
#include  <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include "date.h"
#include "Request.h"
#include "extio.h"
#include "functions_client.h"

#include  <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>

//https://stackoverflow.com/questions/970979/what-are-the-differences-between-poll-and-select
//https://beesbuzz.biz/code/5739-The-problem-with-select-vs-poll
//diafaneies leitourgikwn k.Deli
pthread_t * tid_of_autonomous_process;

int _workers;

char ** __commands;
char * _serverip;
int _port;
int _total_commands;

char buffer[5000];

void catchinterrupt_client(int signo) {
    strcpy(buffer, "exit\n");
}

void loadfileClient(char * queryfile, int * total_commands, char *** commands) {
    int j = 0;
    char filename[PATH_MAX];

    if (queryfile != NULL) {
        strcpy(filename, queryfile);

        *total_commands = 0;

        FILE * fp = fopen(filename, "r");

        char buffer[1000];

        while (fgets(buffer, sizeof (buffer), fp) != NULL) {
            (*total_commands)++;
        }

        fclose(fp);

        printf("Importing file: %s (lines: %d) \n", filename, *total_commands);

        char ** array = malloc(sizeof (char*)*(*total_commands));


        fp = fopen(filename, "r");

        while (fgets(buffer, sizeof (buffer), fp) != NULL) {
            array[j++] = strdup(buffer);

            int l = strlen(array[j - 1]);
            array[j - 1][l - 1] = '\0';
        }

        fclose(fp);

        *commands = array;
    }
}

void * start(void *arg) {
    int * id = (int*) arg;
    printf("Worker started with id: %d  \n", *id);

    main_worker_from_pthread_create(id, _total_commands, __commands, _serverip, _port, _workers);


    printf("Worker finished with id: %d \n", *id);

    free(id);

    return 0;
}

void initializeClient(int w, int total_commands, char **commands, char * serverIp, int serverPort) {
    int i;
    _workers = w;
    __commands = commands;
    _total_commands = total_commands;
    _serverip = serverIp;
    _port = serverPort;


    if (commands != NULL) {
        printf("Commands loaded: \n");

        for (i = 0; i < total_commands; i++) {
            printf("Worker[%02d]: Command[%04d] = %s \n", i % w, i, commands[i]);
        }
    }

    printf("---------------------------------------------------\n");


    tid_of_autonomous_process = malloc(sizeof (pthread_t) * w);

    for (i = 0; i < w; i++) {
        int * id = malloc(sizeof (int));
        *id = i;
        pthread_create(&tid_of_autonomous_process[i], NULL, start, id);
    }

    static struct sigaction act;
    act.sa_handler = catchinterrupt_client;
    sigfillset(&(act.sa_mask));
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);

    //    for (i = 0; i < _total_commands_client; i++) {
    //        int worker = i % w;
    //
    //        char * message = malloc(sizeof (char)*b);
    //
    //        memset(message, 0, b);
    //
    //        strcpy(message, _commands_client[i]);
    //
    //        writeall(down_fd[worker], message, b);
    //
    //        free(message);
    //    }


    printf("App initialized \n");
}

void cleanUpDataStructuresClient(int total_commands, char **commands) {
    printf("Structures cleaned\n");
    int i;

    for (i = 0; i < _workers; i++) {
        pthread_join(tid_of_autonomous_process[i], NULL);
    }

    free(tid_of_autonomous_process);

    if (commands != NULL) {
        for (i = 0; i < total_commands; i++) {
            free(commands[i]);
        }

        free(commands);
    }
}

void AggregatorSync() {
    //    int i;
    //    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));
    //
    //    char * buffer = calloc(1, (sizeof (char)*_socketb));
    //
    //    Request request = {0};
    //
    //    strcpy(request.cmd, "sync");
    //
    //    memset(buffer, 0, _socketb);
    //
    //    memcpy(buffer, &request, sizeof (Request));
    //
    //    for (i = 0; i < _workers; i++) {
    //        int target = i;
    //
    //        int down = down_fd[target];
    //
    //        writeall(down, buffer, _socketb);
    //    }
    //
    //    for (i = 0; i < _workers; i++) {
    //        int target = i;
    //
    //        int up = up_fd[target];
    //
    //        fdarray[i].fd = up;
    //        fdarray[i].events = POLLIN;
    //    }
    //
    //
    //    int count_down = _workers;
    //
    //    while (count_down > 0) {
    //        int rc = poll(fdarray, _workers, -1);
    //
    //        if (rc == 0) {
    //            printf("pool timeout \n");
    //            continue;
    //        }
    //        if (rc == -1) {
    //            perror("poll");
    //            break;
    //        }
    //
    //        if (rc > 0) {
    //            count_down = count_down - rc;
    //
    //            for (i = 0; i < _workers; i++) {
    //                if (fdarray[i].revents == POLLIN) {
    //
    //                    readall(fdarray[i].fd, buffer, _socketb);
    //
    //                    Response response = {0};
    //                    memcpy(&response, buffer, sizeof (Response));
    //                }
    //            }
    //        }
    //    }
    //
    //    free(fdarray);
    //    free(buffer);
}

void requestDiseaseFrequencyWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);

    // receive response
    readall(fd, buffer, socketb);

    Response res = {0};

    memcpy(&res, buffer, sizeof (Response));

    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: %d\n", res.number);
    //    printf(" *** Client q       : %d\n", q);
    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void requestDiseaseFrequencyWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);

    // receive response
    readall(fd, buffer, socketb);

    Response res = {0};

    memcpy(&res, buffer, sizeof (Response));

    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: %d\n", res.number);
    //    printf(" *** Client q       : %d\n", q);
    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void requestPatientRecord(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * id) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);

    // receive response
    readall(fd, buffer, socketb);

    Response res = {0};

    memcpy(&res, buffer, sizeof (Response));

    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: %s\n", res.buffer);
    //    printf(" *** Client q       : %d\n", q);
    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void patientAdmissionsWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);


    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: \n");

    while (1) {
        // receive response
        if (readall(fd, buffer, socketb) <0 ) {
            break;
        }

        Response res = {0};

        memcpy(&res, buffer, sizeof (Response));

        if (res.buffer[0] == '\0') {
            break;
        } else {
            printf("\t%s %d\n", res.buffer, res.number);
        }
    }

    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void patientAdmissionsWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);

    // receive response
    readall(fd, buffer, socketb);

    Response res = {0};

    memcpy(&res, buffer, sizeof (Response));

    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: %d\n", res.number);
    //    printf(" *** Client q       : %d\n", q);
    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void patientDischargesWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);


    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: \n");

    while (1) {
        // receive response
        if (readall(fd, buffer, socketb) <= 0) {
            break;
        }

        Response res = {0};

        memcpy(&res, buffer, sizeof (Response));

        if (res.buffer[0] == '\0') {
            break;
        } else {
            printf("\t%s %d\n", res.buffer, res.number);
        }
    }

    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void patientDischargesWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);

    // receive response
    readall(fd, buffer, socketb);

    Response res = {0};

    memcpy(&res, buffer, sizeof (Response));

    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: %d\n", res.number);
    //    printf(" *** Client q       : %d\n", q);
    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void topKAgeRanges(char * cmd, pthread_mutex_t * lock, int fd, int socketb,char * ktoken, char * countrytoken, char * diseasetoken, char * dateA, char * dateB) {
    char * buffer = calloc(1, (sizeof (char)*socketb));

    Request request = {0};

    strcpy(request.cmd, cmd);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    // send request
    writeall(fd, buffer, socketb);

    // receive response
    readall(fd, buffer, socketb);

    Response res = {0};

    memcpy(&res, buffer, sizeof (Response));

    pthread_mutex_lock(lock);
    printf("###############################################################\n");
    printf("\t*** Client request : %s\n", cmd);
    printf("\t*** Client response: %s\n", res.buffer);
    //    printf(" *** Client q       : %d\n", q);
    printf("###############################################################\n");
    pthread_mutex_unlock(lock);

    free(buffer);
}

void interactWithUserClient() {

    //    int i;
    //
    //    AggregatorSync();
    //
    //
    //    sigset_t set1;
    //
    //    sigfillset(&set1);
    //
    //
    //    while (1) {
    //        printf("Aggregator - Type command (PID:%d) (buffer=%s): ", getpid(), buffer);
    //
    //        sigprocmask(SIG_UNBLOCK, &set1, NULL);
    //
    //        fgets(buffer, sizeof (buffer), stdin);
    //
    //        sigprocmask(SIG_BLOCK, &set1, NULL);
    //
    //        int length = strlen(buffer);
    //        buffer[length - 1] = '\0';
    //
    //        if (length < 3) {
    //            printf("unknown first token \n");
    //            continue;
    //        }
    //
    //        char * firsttoken = strtok(buffer, " ");
    //
    //        if (strcmp(firsttoken, "searchPatientRecord") == 0 || strcmp(firsttoken, "sp") == 0) {
    //            char * secondtoken = strtok(NULL, " ");
    //
    //            if (secondtoken == NULL) {
    //                printf("second token invalid \n");
    //            } else {
    //                char * tokenA = secondtoken;
    //
    //                if (tokenA != NULL) {
    //                    requestPatientRecord(tokenA);
    //                }
    //            }
    //        } else if (strcmp(firsttoken, "listCountries") == 0 || strcmp(firsttoken, "li") == 0) {
    //            for (i = 0; i < _total_commands_client; i++) {
    //                int worker = i % _workers;
    //                printf(" - %s %d \n", _commands_client[i], tid_of_autonomous_process[worker]);
    //            }
    //            printf("\n");
    //        } else if (strcmp(firsttoken, "diseaseFrequency") == 0 || strcmp(firsttoken, "dfreq") == 0) {
    //            //printf("first token correct : diseaseFrequency \n");
    //
    //            char * diseasetoken = strtok(NULL, " ");
    //            char * secondtoken = strtok(NULL, " ");
    //            char * thirdtoken = strtok(NULL, " ");
    //            char * countrytoken = strtok(NULL, " ");
    //
    //            if (diseasetoken == NULL) {
    //                printf("invalid disease \n");
    //                _errors_aggregator++;
    //            }
    //            if (secondtoken == NULL) {
    //                printf("Invalid date \n");
    //                _errors_aggregator++;
    //            } else {
    //                char * dateA = secondtoken;
    //                if (dateA != NULL) {
    //
    //                    char * dateB = thirdtoken;
    //                    if (dateB == NULL) {
    //                        printf("third token invalid \n");
    //                        _errors_aggregator++;
    //                    } else if (countrytoken == NULL) {
    //                        requestDiseaseFrequencyWithoutCountry(diseasetoken, secondtoken, thirdtoken);
    //                    } else {
    //                        requestDiseaseFrequencyWithCountry(diseasetoken, secondtoken, thirdtoken, countrytoken);
    //                    }
    //                }
    //            }
    //        } else if (strcmp(firsttoken, "topk-AgeRanges") == 0 || strcmp(firsttoken, "tar") == 0) {
    //            // printf("first token correct : topk_AgeRanges \n");
    //
    //            char * ktoken = strtok(NULL, " ");
    //            char * countrytoken = strtok(NULL, " ");
    //            char * diseasetoken = strtok(NULL, " ");
    //            char * secondtoken = strtok(NULL, " ");
    //            char * thirdtoken = strtok(NULL, " ");
    //
    //            if (ktoken == NULL) {
    //                printf("invalid k \n");
    //                _errors_aggregator++;
    //            } else if (countrytoken == NULL) {
    //                printf("invalid countrytoken \n");
    //                _errors_aggregator++;
    //            } else if (diseasetoken == NULL) {
    //                printf("invalid disease \n");
    //                _errors_aggregator++;
    //            } else if (secondtoken == NULL) {
    //                printf("Invalid date \n");
    //                _errors_aggregator++;
    //            } else if (thirdtoken == NULL) {
    //                printf("Invalid date \n");
    //                _errors_aggregator++;
    //            } else {
    //                //    char * dateA = secondtoken;
    //                //    char * dateB = thirdtoken;
    //
    //                topKAgeRanges(ktoken, countrytoken, diseasetoken, secondtoken, thirdtoken);
    //            }
    //        } else if (strcmp(firsttoken, "numPatientAdmissions") == 0 || strcmp(firsttoken, "npa") == 0) {
    //            // printf("first token correct : numPatientAdmissions \n");
    //
    //            char * diseasetoken = strtok(NULL, " ");
    //            char * secondtoken = strtok(NULL, " ");
    //            char * thirdtoken = strtok(NULL, " ");
    //            char * countrytoken = strtok(NULL, " ");
    //
    //            if (diseasetoken == NULL) {
    //                printf("invalid disease \n");
    //                _errors_aggregator++;
    //            }
    //            if (secondtoken == NULL) {
    //                printf("Invalid date \n");
    //                _errors_aggregator++;
    //            } else {
    //                char * dateA = secondtoken;
    //                if (dateA != NULL) {
    //
    //                    char * dateB = thirdtoken;
    //                    if (dateB == NULL) {
    //                        printf("third token invalid \n");
    //                        _errors_aggregator++;
    //                    } else if (countrytoken == NULL) {
    //                        // for (i = 0; i < _total_countries_aggregator; i++) {
    //                        PatientAdmissionsWithoutCountry(diseasetoken, dateA, dateB);
    //                        // }
    //                    } else {
    //                        PatientAdmissionsWithCountry(diseasetoken, dateA, dateB, countrytoken);
    //                    }
    //                }
    //            }
    //        } else if (strcmp(firsttoken, "numPatientDischarges") == 0 || strcmp(firsttoken, "npd") == 0) {
    //            //  printf("first token correct : numPatientDischarges \n");
    //
    //            char * diseasetoken = strtok(NULL, " ");
    //            char * secondtoken = strtok(NULL, " ");
    //            char * thirdtoken = strtok(NULL, " ");
    //            char * countrytoken = strtok(NULL, " ");
    //
    //            if (diseasetoken == NULL) {
    //                printf("invalid disease \n");
    //                _errors_aggregator++;
    //            }
    //            if (secondtoken == NULL) {
    //                printf("Invalid date \n");
    //                _errors_aggregator++;
    //            } else {
    //                char * dateA = secondtoken;
    //                if (dateA != NULL) {
    //
    //                    char * dateB = thirdtoken;
    //                    if (dateB == NULL) {
    //                        printf("third token invalid \n");
    //                        _errors_aggregator++;
    //                    } else if (countrytoken == NULL) {
    //                        //for (i = 0; i < _total_countries_aggregator; i++) {
    //                        PatientDisChargesWithoutCountry(diseasetoken, dateA, dateB);
    //                        // }
    //                    } else {
    //                        PatientDisChargesWithCountry(diseasetoken, dateA, dateB, countrytoken);
    //                    }
    //                }
    //            }
    //        }
    //
    //        if (strncmp(buffer, "exit", 4) == 0) {
    //            break;
    //        }
    //    }

}
//// --------------------------------------------------------------------------