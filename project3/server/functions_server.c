#include  <poll.h>
#include  <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <netdb.h>          /* gethostbyaddr */
#include <arpa/inet.h>
#include <signal.h>
#include "functions_worker.h"
#include "date.h"
#include "Request.h"
#include "extio.h"
#include "functions_server.h"
#include "queue.h"
#include "listlist.h"

#include  <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>

//https://stackoverflow.com/questions/970979/what-are-the-differences-between-poll-and-select
//https://beesbuzz.biz/code/5739-The-problem-with-select-vs-poll
//diafaneies leitourgikwn k.Deli
//https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr
int _queueb = 0;

pthread_t * tid_of_autonomous_process;

Queue queue;

int _threads;

int client_fd;
int worker_fd;

char client_socket_ipbuffer[20];
char worker_socket_ipbuffer[20];

char buffer[5000];

void catchinterrupt_aggregator(int signo) {
    printf("Server signalled for shutdown \n");
    strcpy(buffer, "exit\n");
}

void initializeServer(int q, int s, int queue_b, int w, int b, int total_countries, char **countries) {
    _queueb = queue_b;
    _threads = w;

    client_fd = createSocket(client_socket_ipbuffer, q);
    worker_fd = createSocket(worker_socket_ipbuffer, s);

    tid_of_autonomous_process = malloc(sizeof (pthread_t) * w);


    //
    //    for (i = 0; i < w; i++) {
    //        tid_of_autonomous_process[i] = fork();
    ///
    //        if (tid_of_autonomous_process[i] == 0) {
    //            printf("Worker start\n");
    //            printf("Worker finish\n");
    //
    //            client_fd[i] = open(aggregator_down[i], O_RDONLY);
    //            worker_fd[i] = open(aggregator_up[i], O_WRONLY);
    //
    //            main_worker_from_fork(aggregator_inputdir, b, client_fd[i], worker_fd[i]);
    //
    //            close(client_fd[i]);
    //            close(worker_fd[i]);
    //            exit(0);
    //        }
    //    }

    static struct sigaction act;
    act.sa_handler = catchinterrupt_aggregator;
    sigfillset(&(act.sa_mask));
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);

    printf("Server initialized \n");

    printf("Client incoming address: %s:%d \n", client_socket_ipbuffer, q);
    printf("Worker incoming address: %s:%d \n", worker_socket_ipbuffer, s);
}

void * start(void *arg) {
    int * id = (int*) arg;
    printf("Worker started with id: %d  \n", *id);

    main_worker_from_pthread_create(*id, _threads, &queue);


    printf("Worker finished with id: %d \n", *id);

    free(id);

    return 0;
}

void createThreadPool() {
    int i;
    for (i = 0; i < _threads; i++) {
        int * id = malloc(sizeof (int));
        *id = i;
        pthread_create(&tid_of_autonomous_process[i], NULL, start, id);
    }
}

void cleanUpThreadPool() {
    int i;

    for (i = 0; i < _threads; i++) {
        placeQ(&queue, -1);
    }

    notifyConsumers(&queue);

    for (i = 0; i < _threads; i++) {
        pthread_join(tid_of_autonomous_process[i], NULL);
    }
}

void createDataStructuresServer() {
    printf("Structures created \n");

    initializeQ(&queue, _queueb);

    initializeLL();
}

void cleanUpDataStructuresServer() {
    printf("Structures cleaned\n");
    //int i;

    close(client_fd);
    close(worker_fd);

    // -------------------------------------------------------

    // for (i = 0; i < _threads; i++) {
    //     pthread_join(tid_of_autonomous_process[i], NULL);
    // }

    cleanupQ(&queue);

    cleanupLL();

    free(tid_of_autonomous_process);

    //    cleanupLL();

}

void PatientAdmissionsWithoutCountry(char * diseasetoken, char *secondtoken, char * thirdtoken) {
    //    int i;
    //    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));
    //
    //    char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //    Request request = {0};
    //
    //    strcpy(request.cmd, "numPatientAdmissions");
    //    strcpy(request.firsttoken, diseasetoken);
    //    strcpy(request.secondtoken, secondtoken);
    //    strcpy(request.thirdtoken, thirdtoken);
    //
    //    memset(buffer, 0, _pipeb);
    //
    //    memcpy(buffer, &request, sizeof (Request));
    //
    //    for (i = 0; i < _workers; i++) {
    //        int target = i;
    //
    //        int down = client_fd[target];
    //
    //        writeall(down, buffer, _pipeb);
    //    }
    //
    //    for (i = 0; i < _workers; i++) {
    //        int target = i;
    //
    //        int up = worker_fd[target];
    //
    //        fdarray[i].fd = up;
    //        fdarray[i].events = POLLIN;
    //    }
    //
    //
    //    int count_down = _total_countries_aggregator;
    //
    //    while (count_down > 0) {
    //        int rc = poll(fdarray, _workers, -1);
    //
    //        if (rc == 0) {
    //            printf("poll timeout \n");
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
    //                    readall(fdarray[i].fd, buffer, _pipeb);
    //
    //                    Response response = {0};
    //                    memcpy(&response, buffer, sizeof (Response));
    //
    //                    if (response.buffer[0] == '\0') {
    //                        count_down -= response.number;
    //                        _errors_aggregator++;
    //                    } else {
    //                        printf("%s %d\n", response.buffer, response.number);
    //                        _successes_aggregator++;
    //                    }
    //                }
    //            }
    //        }
    //    }
    //
    //    free(fdarray);
    //    free(buffer);
}

void PatientDisChargesWithoutCountry(char * diseasetoken, char * secondtoken, char * thirdtoken) {
    //    int i;
    //    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));
    //
    //    char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //    Request request = {0};
    //
    //    strcpy(request.cmd, "numPatientDischarges");
    //    strcpy(request.firsttoken, diseasetoken);
    //    strcpy(request.secondtoken, secondtoken);
    //    strcpy(request.thirdtoken, thirdtoken);
    //
    //    memset(buffer, 0, _pipeb);
    //
    //    memcpy(buffer, &request, sizeof (Request));
    //
    //    for (i = 0; i < _workers; i++) {
    //        int target = i;
    //
    //        int down = client_fd[target];
    //
    //        writeall(down, buffer, _pipeb);
    //    }
    //
    //    for (i = 0; i < _workers; i++) {
    //        int target = i;
    //
    //        int up = worker_fd[target];
    //
    //        fdarray[i].fd = up;
    //        fdarray[i].events = POLLIN;
    //    }
    //
    //
    //    int count_down = _total_countries_aggregator;
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
    //                    readall(fdarray[i].fd, buffer, _pipeb);
    //
    //                    Response response = {0};
    //                    memcpy(&response, buffer, sizeof (Response));
    //
    //                    if (response.buffer[0] == '\0') {
    //                        count_down -= response.number;
    //                        _errors_aggregator++;
    //                    } else {
    //                        printf("%s %d\n", response.buffer, response.number);
    //                        _successes_aggregator++;
    //                    }
    //                }
    //            }
    //        }
    //    }
    //
    //    free(fdarray);
    //    free(buffer);
}

void requestDiseaseFrequencyWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken) {
    int _workers = numWorkers();
    int i;

    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    int * worker_fds = calloc(_workers, sizeof (int));

    connectToAll(worker_fds);

    char * buffer = calloc(socketb, (sizeof (char)));

    Request request = {0};

    strcpy(request.cmd, "diseaseFrequency");
    strcpy(request.firsttoken, diseasetoken);
    strcpy(request.secondtoken, secondtoken);
    strcpy(request.thirdtoken, thirdtoken);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = worker_fds[target];

        writeall(down, buffer, socketb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = worker_fds[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }


    int sum = 0;

    int count_down = _workers;

    while (count_down > 0) {
        int rc = poll(fdarray, _workers, -1);

        if (rc == 0) {
            printf("pool timeout \n");
            continue;
        }
        if (rc == -1) {
            perror("poll");
            break;
        }

        if (rc > 0) {
            count_down = count_down - rc;

            for (i = 0; i < _workers; i++) {
                if (fdarray[i].revents == POLLIN && fdarray[i].events != 0) {
                    readall(fdarray[i].fd, buffer, socketb);

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));

                    sum = sum + response.number;

                    fdarray[i].events = 0;
                    fdarray[i].revents = 0;
                }
            }
        }
    }

    printf("Server print: %s => %d\n", cmd, sum);


    free(fdarray);

    Response response = {0};
    response.number = sum;

    memcpy(buffer, &response, sizeof (Response));

    writeall(fd, buffer, socketb);

    for (i = 0; i < _workers; i++) {
        close(worker_fds[i]);
    }

    free(worker_fds);

    free(buffer);
}

void requestDiseaseFrequencyWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    int * worker_fds = calloc(1, sizeof (int));

    connectSingle(worker_fds, countrytoken);

    if (worker_fds[0] == -1) {
        char * buffer = calloc(socketb, (sizeof (char)));

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %d\n", cmd, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    } else {
        char * buffer = calloc(socketb, (sizeof (char)));

        Request request = {0};

        strcpy(request.cmd, "diseaseFrequency");
        strcpy(request.firsttoken, diseasetoken);
        strcpy(request.secondtoken, secondtoken);
        strcpy(request.thirdtoken, thirdtoken);
        strcpy(request.fourthtoken, countrytoken);

        memset(buffer, 0, socketb);

        memcpy(buffer, &request, sizeof (Request));

        int target = 0;

        int down = worker_fds[target];
        int up = worker_fds[target];

        writeall(down, buffer, socketb);

        readall(up, buffer, socketb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %d\n", cmd, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    }
}

void requestPatientRecord(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * id) {
    int _workers = numWorkers();
    int i;

    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    int * worker_fds = calloc(_workers, sizeof (int));

    connectToAll(worker_fds);

    char * buffer = calloc(socketb, (sizeof (char)));

    Request request = {0};

    strcpy(request.cmd, "searchPatientRecord");
    strcpy(request.firsttoken, id);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = worker_fds[target];

        writeall(down, buffer, socketb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = worker_fds[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }


    int count_down = _workers;

    Response response = {0};
    strcpy(response.buffer, "record not found");

    while (count_down > 0) {
        int rc = poll(fdarray, _workers, -1);

        if (rc == 0) {
            printf("pool timeout \n");
            continue;
        }
        if (rc == -1) {
            perror("poll");
            break;
        }

        if (rc > 0) {
            count_down = count_down - rc;

            for (i = 0; i < _workers; i++) {
                if (fdarray[i].revents == POLLIN && fdarray[i].events != 0) {
                    readall(fdarray[i].fd, buffer, socketb);

                    Response response2 = {0};
                    memcpy(&response2, buffer, sizeof (Response));

                    if (strcmp(response2.buffer, "record not found \n") != 0) {
                        strcpy(response.buffer, response2.buffer);
                    }

                    fdarray[i].events = 0;
                    fdarray[i].revents = 0;
                }
            }
        }
    }

    printf("Server print: %s => %s\n", cmd, response.buffer);

    free(fdarray);

    memcpy(buffer, &response, sizeof (Response));

    writeall(fd, buffer, socketb);

    for (i = 0; i < _workers; i++) {
        close(worker_fds[i]);
    }

    free(worker_fds);

    free(buffer);
}

void patientAdmissionsWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken) {
    int _workers = numWorkers();
    int i;

    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    int * worker_fds = calloc(_workers, sizeof (int));

    connectToAll(worker_fds);

    char * buffer = calloc(socketb, (sizeof (char)));

    Request request = {0};

    strcpy(request.cmd, "numPatientAdmissions");
    strcpy(request.firsttoken, diseasetoken);
    strcpy(request.secondtoken, secondtoken);
    strcpy(request.thirdtoken, thirdtoken);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = worker_fds[target];

        writeall(down, buffer, socketb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = worker_fds[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }


    int count_down = _workers;

    while (count_down > 0) {
        int rc = poll(fdarray, _workers, -1);

        if (rc == 0) {
            break;
        }
        if (rc == -1) {
            perror("poll");
            break;
        }

        if (rc > 0) {
            for (i = 0; i < _workers; i++) {
                if (fdarray[i].revents == POLLIN && fdarray[i].events != 0) {
                    if (readall(fdarray[i].fd, buffer, socketb) <= 0) {
                        count_down -= 1;
                        fdarray[i].events = 0;
                        fdarray[i].revents = 0;
                        continue;
                    }

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));

                    if (response.buffer[0] == '\0') {
                        count_down -= 1;
                        fdarray[i].events = 0;
                        fdarray[i].revents = 0;
                    } else {
                        writeall(fd, buffer, socketb);

                        printf("forward: %s %d\n", response.buffer, response.number);
                    }
                }
            }
        }
    }

    free(fdarray);

    Response response = {0};
    response.buffer[0] = '\0';
    memcpy(buffer, &response, sizeof (Response));

    writeall(fd, buffer, socketb);

    for (i = 0; i < _workers; i++) {

        close(worker_fds[i]);
    }

    free(worker_fds);

    free(buffer);
}

void patientAdmissionsWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    int * worker_fds = calloc(1, sizeof (int));

    connectSingle(worker_fds, countrytoken);

    if (worker_fds[0] == -1) {
        char * buffer = calloc(socketb, (sizeof (char)));

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %d\n", cmd, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    } else {

        char * buffer = calloc(socketb, (sizeof (char)));

        Request request = {0};

        strcpy(request.cmd, "diseaseFrequency");
        strcpy(request.firsttoken, diseasetoken);
        strcpy(request.secondtoken, secondtoken);
        strcpy(request.thirdtoken, thirdtoken);
        strcpy(request.fourthtoken, countrytoken);

        memset(buffer, 0, socketb);

        memcpy(buffer, &request, sizeof (Request));

        int target = 0;

        int down = worker_fds[target];
        int up = worker_fds[target];

        writeall(down, buffer, socketb);

        readall(up, buffer, socketb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %s %d\n", cmd, response.buffer, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    }
}

void patientDischargesWithCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    int * worker_fds = calloc(1, sizeof (int));

    connectSingle(worker_fds, countrytoken);

    if (worker_fds[0] == -1) {
        char * buffer = calloc(socketb, (sizeof (char)));

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %d\n", cmd, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    } else {

        char * buffer = calloc(socketb, (sizeof (char)));

        Request request = {0};

        strcpy(request.cmd, "numPatientDischarges");
        strcpy(request.firsttoken, diseasetoken);
        strcpy(request.secondtoken, secondtoken);
        strcpy(request.thirdtoken, thirdtoken);
        strcpy(request.fourthtoken, countrytoken);

        memset(buffer, 0, socketb);

        memcpy(buffer, &request, sizeof (Request));

        int target = 0;

        int down = worker_fds[target];
        int up = worker_fds[target];

        writeall(down, buffer, socketb);

        readall(up, buffer, socketb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %s %d\n", cmd, response.buffer, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    }
}

void patientDischargesWithoutCountry(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * diseasetoken, char * secondtoken, char * thirdtoken) {
    int _workers = numWorkers();
    int i;

    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    int * worker_fds = calloc(_workers, sizeof (int));

    connectToAll(worker_fds);

    char * buffer = calloc(socketb, (sizeof (char)));

    Request request = {0};

    strcpy(request.cmd, "numPatientDischarges");
    strcpy(request.firsttoken, diseasetoken);
    strcpy(request.secondtoken, secondtoken);
    strcpy(request.thirdtoken, thirdtoken);

    memset(buffer, 0, socketb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = worker_fds[target];

        writeall(down, buffer, socketb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = worker_fds[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }


    int count_down = _workers;

    while (count_down > 0) {
        int rc = poll(fdarray, _workers, -1);

        if (rc == 0) {
            break;
        }
        if (rc == -1) {
            perror("poll");
            break;
        }

        if (rc > 0) {
            for (i = 0; i < _workers; i++) {
                if (fdarray[i].revents == POLLIN && fdarray[i].events != 0) {
                    if (readall(fdarray[i].fd, buffer, socketb) <= 0) {
                        count_down -= 1;
                        fdarray[i].events = 0;
                        fdarray[i].revents = 0;
                        continue;
                    }

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));

                    if (response.buffer[0] == '\0') {
                        count_down -= 1;
                        fdarray[i].events = 0;
                        fdarray[i].revents = 0;
                    } else {
                        writeall(fd, buffer, socketb);

                        printf("forward: %s %d\n", response.buffer, response.number);
                    }
                }
            }
        }
    }

    free(fdarray);

    Response response = {0};
    response.buffer[0] = '\0';
    memcpy(buffer, &response, sizeof (Response));

    writeall(fd, buffer, socketb);

    for (i = 0; i < _workers; i++) {

        close(worker_fds[i]);
    }

    free(worker_fds);

    free(buffer);
}

void topKAgeRanges(char * cmd, pthread_mutex_t * lock, int fd, int socketb, char * ktoken, char * countrytoken, char * diseasetoken, char * dateA, char * dateB) {
    int * worker_fds = calloc(1, sizeof (int));

    connectSingle(worker_fds, countrytoken);

    if (worker_fds[0] == -1) {
        char * buffer = calloc(socketb, (sizeof (char)));

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %d\n", cmd, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    } else {
        char * buffer = calloc(socketb, (sizeof (char)));

        Request request = {0};

        strcpy(request.cmd, "topk-AgeRanges");
        strcpy(request.firsttoken, ktoken);
        strcpy(request.secondtoken, countrytoken);
        strcpy(request.thirdtoken, diseasetoken);
        strcpy(request.fourthtoken, dateA);
        strcpy(request.fifthtoken, dateB);


        memset(buffer, 0, socketb);

        memcpy(buffer, &request, sizeof (Request));

        int target = 0;

        int down = worker_fds[target];
        int up = worker_fds[target];

        writeall(down, buffer, socketb);

        readall(up, buffer, socketb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Server print: %s => %d\n", cmd, response.number);

        memcpy(buffer, &response, sizeof (Response));

        writeall(fd, buffer, socketb);

        close(worker_fds[0]);

        free(worker_fds);

        free(buffer);
    }
}

void interactWithClientsAndWorkers() {
    struct sockaddr_in client;
    socklen_t clientlen = sizeof (client);
    struct sockaddr *clientptr = (struct sockaddr *) &client;
    int i;
    char buffer[5000];

    sigset_t set1;

    sigfillset(&set1);

    struct pollfd * fdarray = calloc(3, sizeof (struct pollfd));

    fdarray[0].fd = client_fd;
    fdarray[1].fd = worker_fd;
    fdarray[2].fd = 0;

    for (i = 0; i < 3; i++) {
        fdarray[i].events = POLLIN;
        fdarray[i].revents = 0;
    }


    while (1) {
        printf("Server - Waiting for clients or workers (polling) ... \n");

        int rc = poll(fdarray, 3, -1);

        if (rc == 0) {
            printf("pool timeout \n");
            continue;
        }
        if (rc == -1) {
            perror("poll");
            break;
        }

        if (rc > 0) {
            int newsock;


            if (fdarray[0].revents == POLLIN) { // client: receive command
                if ((newsock = accept(client_fd, clientptr, &clientlen)) < 0) {
                    perror("accept");
                    exit(1);
                }

                printf("Client connected \n");

                placeQ(&queue, newsock);

                notifyConsumers(&queue);
            }

            if (fdarray[1].revents == POLLIN) { // worker: receive IP, port, countries etc
                if ((newsock = accept(worker_fd, clientptr, &clientlen)) < 0) {
                    perror("accept");
                    exit(1);
                }

                printf("Worker connected \n");

                placeQ(&queue, newsock);

                notifyConsumers(&queue);
            }

            if (fdarray[2].revents == POLLIN) {
                fgets(buffer, sizeof (buffer), stdin);
                int length = strlen(buffer);
                buffer[length - 1] = '\0';

                if (length == 0) {
                    printf("unknown first token \n");
                    continue;
                }

                char * pch = NULL;
                char * firsttoken = strtok_r(buffer, " ", &pch);

                if (strcmp(firsttoken, "listWorkers") == 0 || strcmp(firsttoken, "lw") == 0) {
                    printf("Workers: \n");

                    printLL();
                }

                if (strcmp(firsttoken, "exit") == 0 || strcmp(firsttoken, "e") == 0) {

                    break;
                }
            }
        }
    }

}


//// --------------------------------------------------------------------------

int createSocket(char ipbuffer[20], int port) {
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    socklen_t serverlen = sizeof (server);

    int _listeningSocket;

    if ((_listeningSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("#### socket");
        exit(1);
    }

    if (setsockopt(_listeningSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof (int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    if (bind(_listeningSocket, serverptr, sizeof (server)) < 0) {
        perror("#### bind");
        exit(2);
    }

    if (listen(_listeningSocket, 1337) < 0) {
        perror("#### listen");
        exit(3);
    }

    if (getsockname(_listeningSocket, serverptr, &serverlen) < 0) {
        perror("#### getsockname");
        exit(4);
    }

    char hostbuffer[256];

    gethostname(hostbuffer, sizeof (hostbuffer));

    struct hostent *host_entry = gethostbyname(hostbuffer);

    char * tempIPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

    strcpy(ipbuffer, tempIPbuffer);

    return _listeningSocket;
}
