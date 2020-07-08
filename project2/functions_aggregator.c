#include  <poll.h>
#include  <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include "functions_worker.h"
#include "listlist.h"
#include "hashdisease.h"
#include "hashcountry.h"
#include "DiseaseData.h"
#include "CountryData.h"
#include "date.h"
#include "quicksort.h"
#include "Request.h"
#include "extio.h"

#include  <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

//https://stackoverflow.com/questions/970979/what-are-the-differences-between-poll-and-select
//https://beesbuzz.biz/code/5739-The-problem-with-select-vs-poll
//diafaneies leitourgikwn k.Deli
int _pipeb = 0;
int _total_countries_aggregator;
char ** _countries_aggregator;
char logfile_name_aggregator[4000] = "log_file_aggregator.";

int _successes_aggregator = 0, _errors_aggregator = 0;
char ** aggregator_up;
char ** aggregator_down;
pid_t * pid_of_autonomous_process;
char * aggregator_inputdir;

int _workers;

int * down_fd;
int * up_fd;

char buffer[5000];

void catchinterrupt_aggregator(int signo) {
    strcpy(buffer, "exit\n");
}

void loaddirAggregator(char * inputdir, int * total_countries, char *** countries) {
    DIR *dir_ptr;
    struct dirent *direntp;
    int j;
    char dirname[PATH_MAX];

    if (inputdir == NULL) {
        inputdir = "inputdir";
    }

    aggregator_inputdir = inputdir;

    strcpy(dirname, inputdir);

    if ((dir_ptr = opendir(dirname)) == NULL) {
        fprintf(stderr, "cannot open %s \n", dirname);
        exit(1);
    } else {
        *total_countries = 0;

        while ((direntp = readdir(dir_ptr)) != NULL) {
            if (direntp->d_name[0] == '.') {
                continue;
            }
            (*total_countries)++;
        }

        closedir(dir_ptr);

        char ** array = malloc(sizeof (char*)*(*total_countries));

        dir_ptr = opendir(dirname);

        j = 0;
        while ((direntp = readdir(dir_ptr)) != NULL) {
            if (direntp->d_name[0] == '.') {
                continue;
            }
            array[j++] = strdup(direntp->d_name);
        }

        closedir(dir_ptr);

        quicksort_entry(array, (*total_countries), 2);

        printf("countries: \n");

        for (j = 0; j < (*total_countries); j++) {
            printf("- %s\n", array[j]);
        }

        (*countries) = array;
    }
}

void initializeAggregator(int w, int b, int total_countries, char **countries) {
    int i;
    _pipeb = b;
    _total_countries_aggregator = total_countries;
    _countries_aggregator = countries;
    _workers = w;

    srand(time(0));

    char buffer2[4000];
    sprintf(buffer2, "%d", getpid());

    strcat(logfile_name_aggregator, buffer2);


    aggregator_down = malloc(sizeof (char*)*w);
    aggregator_up = malloc(sizeof (char*)*w);

    for (i = 0; i < w; i++) {
        aggregator_up[i] = malloc(100);
        aggregator_down[i] = malloc(100);
    }

    for (i = 0; i < w; i++) {
        do {
            int pipe_id = rand();
            sprintf(aggregator_down[i], "%d.pipe", pipe_id);
        } while (mkfifo(aggregator_down[i], 0666) == -1);

        do {
            int pipe_id = rand();
            sprintf(aggregator_up[i], "%d.pipe", pipe_id);
        } while (mkfifo(aggregator_up[i], 0666) == -1);
    }

    pid_of_autonomous_process = malloc(sizeof (pid_t) * w);
    down_fd = malloc(sizeof (int) * w);
    up_fd = malloc(sizeof (int) * w);

    for (i = 0; i < w; i++) {
        pid_of_autonomous_process[i] = fork();

        if (pid_of_autonomous_process[i] == 0) {
            printf("Worker start\n");
            printf("Worker finish\n");

            down_fd[i] = open(aggregator_down[i], O_RDONLY);
            up_fd[i] = open(aggregator_up[i], O_WRONLY);

            main_worker_from_fork(aggregator_inputdir, b, down_fd[i], up_fd[i]);

            close(down_fd[i]);
            close(up_fd[i]);
            exit(0);
        }
    }

    static struct sigaction act;
    act.sa_handler = catchinterrupt_aggregator;
    sigfillset(&(act.sa_mask));
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);

    for (i = 0; i < w; i++) {
        down_fd[i] = open(aggregator_down[i], O_WRONLY);
        up_fd[i] = open(aggregator_up[i], O_RDONLY);
    }

    int * x = malloc(sizeof (int)*w);

    for (i = 0; i < w; i++) {
        x[i] = _total_countries_aggregator / w;
    }

    for (i = 0; i < _total_countries_aggregator % w; i++) {
        x[i]++;
    }

    for (i = 0; i < w; i++) {
        writeall(down_fd[i], &x[i], sizeof (int));
    }

    free(x);

    for (i = 0; i < _total_countries_aggregator; i++) {
        int worker = i % w;

        char * message = malloc(sizeof (char)*b);

        memset(message, 0, b);

        strcpy(message, _countries_aggregator[i]);

        writeall(down_fd[worker], message, b);

        free(message);
    }


    printf("App initialized \n");
}

void createDataStructuresAggregator() {
    printf("Structures created \n");
}

void cleanUpDataStructuresAggregator() {
    printf("Structures cleaned\n");
    int i;

    for (i = 0; i < _workers; i++) {
        close(down_fd[i]);
        close(up_fd[i]);
    }

    FILE * fp = fopen(logfile_name_aggregator, "w+");

    for (i = 0; i < _total_countries_aggregator; i++) {
        fprintf(fp, "%s\n", _countries_aggregator[i]);
    }

    int total = _successes_aggregator + _errors_aggregator;
    fprintf(fp, "Total: %d\n Success: %d\n Error: %d\n", total, _successes_aggregator, _errors_aggregator);
    fclose(fp);

    // -------------------------------------------------------

    for (i = 0; i < _total_countries_aggregator; i++) {
        fprintf(stdout, "%s\n", _countries_aggregator[i]);
    }

    total = _successes_aggregator + _errors_aggregator;
    fprintf(stdout, "Total: %d\n Success: %d\n Error: %d\n", total, _successes_aggregator, _errors_aggregator);

    printf("\nlogfile written to disk by aggregator\n");

    for (i = 0; i < _workers; i++) {
        kill(pid_of_autonomous_process[i], SIGKILL);
    }

    for (i = 0; i < _workers; i++) {
        waitpid(pid_of_autonomous_process[i], NULL, 0);
    }

    for (i = 0; i < _workers; i++) {
        unlink(aggregator_down[i]);
        unlink(aggregator_up[i]);
    }
    for (i = 0; i < _workers; i++) {
        free(aggregator_down[i]);
    }

    free(aggregator_down);

    free(pid_of_autonomous_process);

    //    cleanupLL();


}

void AggregatorSync() {
    int i;
    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    char * buffer = calloc(1, (sizeof (char)*_pipeb));

    Request request = {0};

    strcpy(request.cmd, "sync");

    memset(buffer, 0, _pipeb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = down_fd[target];

        writeall(down, buffer, _pipeb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = up_fd[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }


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
                if (fdarray[i].revents == POLLIN) {

                    readall(fdarray[i].fd, buffer, _pipeb);

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));
                }
            }
        }
    }

    free(fdarray);
    free(buffer);
}

void PatientAdmissionsWithCountry(char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    int i;
    int target = -1;
    for (i = 0; i < _total_countries_aggregator; i++) {
        if (strcmp(_countries_aggregator[i], countrytoken) == 0) {
            // printf("Country found at: %d for worker: %d \n", i, pid_of_autonomous_process[i % _workers]);
            target = i % _workers;
            break;
        }
    }

    if (target != -1) {
        int down = down_fd[target];
        int up = up_fd[target];

        Request request = {0};

        strcpy(request.cmd, "numPatientAdmissions");
        strcpy(request.firsttoken, diseasetoken);
        strcpy(request.secondtoken, secondtoken);
        strcpy(request.thirdtoken, thirdtoken);
        strcpy(request.fourthtoken, countrytoken);

        char * buffer = calloc(1, (sizeof (char)*_pipeb));

        memcpy(buffer, &request, sizeof (Request));

        writeall(down, buffer, _pipeb);

        readall(up, buffer, _pipeb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("Aggregator print: %s %d\n", response.buffer, response.number);

        free(buffer);
        _successes_aggregator++;
    } else {
        printf("Country not valid \n");
        _errors_aggregator++;
    }
}

void PatientAdmissionsWithoutCountry(char * diseasetoken, char *secondtoken, char * thirdtoken) {
    int i;
    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    char * buffer = calloc(1, (sizeof (char)*_pipeb));

    Request request = {0};

    strcpy(request.cmd, "numPatientAdmissions");
    strcpy(request.firsttoken, diseasetoken);
    strcpy(request.secondtoken, secondtoken);
    strcpy(request.thirdtoken, thirdtoken);

    memset(buffer, 0, _pipeb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = down_fd[target];

        writeall(down, buffer, _pipeb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = up_fd[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }


    int count_down = _total_countries_aggregator;

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
                if (fdarray[i].revents == POLLIN) {

                    readall(fdarray[i].fd, buffer, _pipeb);

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));
                    
                    if (response.buffer[0] == '\0') {
                        count_down -= response.number;
                        _errors_aggregator++;
                    } else {                    
                        printf("%s %d\n", response.buffer, response.number);
                        _successes_aggregator++;
                    }
                }
            }
        }
    }

    free(fdarray);
    free(buffer);
}

void PatientDisChargesWithCountry(char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    int i;
    int target = -1;
    for (i = 0; i < _total_countries_aggregator; i++) {
        if (strcmp(_countries_aggregator[i], countrytoken) == 0) {
            // printf("Country found at: %d for worker: %d \n", i, pid_of_autonomous_process[i % _workers]);
            target = i % _workers;
            break;
        }
    }

    if (target != -1) {
        int down = down_fd[target];
        int up = up_fd[target];

        Request request = {0};

        strcpy(request.cmd, "numPatientDischarges");
        strcpy(request.firsttoken, diseasetoken);
        strcpy(request.secondtoken, secondtoken);
        strcpy(request.thirdtoken, thirdtoken);
        strcpy(request.fourthtoken, countrytoken);

        char * buffer = calloc(1, (sizeof (char)*_pipeb));

        memset(buffer, 0, _pipeb);

        memcpy(buffer, &request, sizeof (Request));

        writeall(down, buffer, _pipeb);

        readall(up, buffer, _pipeb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));
        if (response.buffer == '\0') {
           _errors_aggregator++;
           }
        else {
            printf("%s %d\n", response.buffer, response.number);
        _successes_aggregator++;
        }
        free(buffer);
    } else {
        printf("Country not valid \n");
        _errors_aggregator++;
    }
}

void PatientDisChargesWithoutCountry(char * diseasetoken, char * secondtoken, char * thirdtoken) {
    int i;
    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    char * buffer = calloc(1, (sizeof (char)*_pipeb));

    Request request = {0};

    strcpy(request.cmd, "numPatientDischarges");
    strcpy(request.firsttoken, diseasetoken);
    strcpy(request.secondtoken, secondtoken);
    strcpy(request.thirdtoken, thirdtoken);

    memset(buffer, 0, _pipeb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = down_fd[target];

        writeall(down, buffer, _pipeb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = up_fd[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }


    int count_down = _total_countries_aggregator;

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
                if (fdarray[i].revents == POLLIN) {

                    readall(fdarray[i].fd, buffer, _pipeb);

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));
                    
                    if (response.buffer[0] == '\0') {
                        count_down -= response.number;
                        _errors_aggregator++;
                    } else {                    
                        printf("%s %d\n", response.buffer, response.number);
                        _successes_aggregator++;
                    }
                }
            }
        }
    }

    free(fdarray);
    free(buffer);
}

void requestDiseaseFrequencyWithoutCountry(char * diseasetoken, char * secondtoken, char * thirdtoken) {
    int i;
    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    char * buffer = calloc(1, (sizeof (char)*_pipeb));

    Request request = {0};

    strcpy(request.cmd, "diseaseFrequency");
    strcpy(request.firsttoken, diseasetoken);
    strcpy(request.secondtoken, secondtoken);
    strcpy(request.thirdtoken, thirdtoken);

    memset(buffer, 0, _pipeb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = down_fd[target];

        writeall(down, buffer, _pipeb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = up_fd[target];

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
                if (fdarray[i].revents == POLLIN) {

                    readall(fdarray[i].fd, buffer, _pipeb);

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));

                    sum = sum + response.number;

                    fdarray[i].events = 0;
                }
            }_successes_aggregator++;
        }
    }

    printf("Aggregator print: %d\n", sum);


    free(fdarray);
    free(buffer);
}

void requestDiseaseFrequencyWithCountry(char * diseasetoken, char * secondtoken, char * thirdtoken, char * countrytoken) {
    int i;
    int target = -1;
    for (i = 0; i < _total_countries_aggregator; i++) {
        if (strcmp(_countries_aggregator[i], countrytoken) == 0) {
            //printf("Country found at: %d for worker: %d \n", i, pid_of_autonomous_process[i % _workers]);
            target = i % _workers;
            break;
        }
    }

    if (target != -1) {
        int down = down_fd[target];
        int up = up_fd[target];

        Request request = {0};

        strcpy(request.cmd, "diseaseFrequency");
        strcpy(request.firsttoken, diseasetoken);
        strcpy(request.secondtoken, secondtoken);
        strcpy(request.thirdtoken, thirdtoken);
        strcpy(request.fourthtoken, countrytoken);

        char * buffer = calloc(1, (sizeof (char)*_pipeb));

        memcpy(buffer, &request, sizeof (Request));

        writeall(down, buffer, _pipeb);

        readall(up, buffer, _pipeb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("%d\n", response.number);

        if (response.status == 1) {
            _successes_aggregator++;
            printf("Status: Success \n");
        } else {
            _errors_aggregator++;
            printf("Status: Fail \n");
        }

        free(buffer);
    } else {
        printf("Country not valid \n");
        _errors_aggregator++;
    }
}

void requestPatientRecord(char * id) {
    int i;
    struct pollfd * fdarray = calloc(_workers, sizeof (struct pollfd));

    char * buffer = calloc(1, sizeof (char)*_pipeb);

    Request request = {0};

    strcpy(request.cmd, "searchPatientRecord");
    strcpy(request.firsttoken, id);

    memset(buffer, 0, _pipeb);

    memcpy(buffer, &request, sizeof (Request));

    for (i = 0; i < _workers; i++) {
        int target = i;

        int down = down_fd[target];

        writeall(down, buffer, _pipeb);
    }

    for (i = 0; i < _workers; i++) {
        int target = i;

        int up = up_fd[target];

        fdarray[i].fd = up;
        fdarray[i].events = POLLIN;
    }

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
                if (fdarray[i].revents == POLLIN) {

                    readall(fdarray[i].fd, buffer, _pipeb);

                    Response response = {0};
                    memcpy(&response, buffer, sizeof (Response));
                    if (response.buffer == '\0') {
                        _errors_aggregator++;
                    }
                    else {
                        printf("Aggregator from worker %d: %s", i, response.buffer);
                    }
                    fdarray[i].events = 0;
                }
            }_successes_aggregator++;
        }
    }

    free(fdarray);

    free(buffer);
}

void topKAgeRanges(char * ktoken, char * countrytoken, char * diseasetoken, char * dateA, char * dateB) {
    int i;
    int target = -1;
    for (i = 0; i < _total_countries_aggregator; i++) {
        if (strcmp(_countries_aggregator[i], countrytoken) == 0) {
            printf("Country found at: %d for worker: %d \n", i, pid_of_autonomous_process[i % _workers]);
            target = i % _workers;
            break;
        }
    }

    if (target != -1) {
        int down = down_fd[target];
        int up = up_fd[target];

        Request request = {0};

        strcpy(request.cmd, "topk-AgeRanges");
        strcpy(request.firsttoken, ktoken);
        strcpy(request.secondtoken, countrytoken);
        strcpy(request.thirdtoken, diseasetoken);
        strcpy(request.fourthtoken, dateA);
        strcpy(request.fifthtoken, dateB);

        char * buffer = calloc(1, (sizeof (char)*_pipeb));

        memset(buffer, 0, _pipeb);

        memcpy(buffer, &request, sizeof (Request));

        writeall(down, buffer, _pipeb);

        readall(up, buffer, _pipeb);

        Response response = {0};
        memcpy(&response, buffer, sizeof (Response));

        printf("%s\n", response.buffer);
        _successes_aggregator++;
        free(buffer);
    } else {
        printf("Country not valid \n");
        _errors_aggregator++;
    }
}

void interactWithUserAggregator() {

    int i;

    AggregatorSync();

    
    sigset_t set1;
    
    sigfillset(&set1);
            

    while (1) {
        printf("Aggregator - Type command (PID:%d) (buffer=%s): ", getpid(), buffer);
       
        sigprocmask(SIG_UNBLOCK, &set1, NULL);
        
        fgets(buffer, sizeof (buffer), stdin);
        
        sigprocmask(SIG_BLOCK, &set1, NULL);

        int length = strlen(buffer);
        buffer[length - 1] = '\0';

        if (length < 3) {
            printf("unknown first token \n");
            continue;
        }
        
        char * firsttoken = strtok(buffer, " ");

        if (strcmp(firsttoken, "searchPatientRecord") == 0 || strcmp(firsttoken, "sp") == 0) {
            char * secondtoken = strtok(NULL, " ");

            if (secondtoken == NULL) {
                printf("second token invalid \n");
            } else {
                char * tokenA = secondtoken;

                if (tokenA != NULL) {
                    requestPatientRecord(tokenA);
                }
            }
        } else if (strcmp(firsttoken, "listCountries") == 0 || strcmp(firsttoken, "li") == 0) {
            for (i = 0; i < _total_countries_aggregator; i++) {
                int worker = i % _workers;
                printf(" - %s %d \n", _countries_aggregator[i], pid_of_autonomous_process[worker]);
            }
            printf("\n");
        } else if (strcmp(firsttoken, "diseaseFrequency") == 0 || strcmp(firsttoken, "dfreq") == 0) {
            //printf("first token correct : diseaseFrequency \n");

            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");

            if (diseasetoken == NULL) {
                printf("invalid disease \n");
                _errors_aggregator++;
            }
            if (secondtoken == NULL) {
                printf("Invalid date \n");
                _errors_aggregator++;
            } else {
                char * dateA = secondtoken;
                if (dateA != NULL) {

                    char * dateB = thirdtoken;
                    if (dateB == NULL) {
                        printf("third token invalid \n");
                        _errors_aggregator++;
                    } else if (countrytoken == NULL) {
                        requestDiseaseFrequencyWithoutCountry(diseasetoken, secondtoken, thirdtoken);
                    } else {
                        requestDiseaseFrequencyWithCountry(diseasetoken, secondtoken, thirdtoken, countrytoken);
                    }
                }
            }
        } else if (strcmp(firsttoken, "topk-AgeRanges") == 0 || strcmp(firsttoken, "tar") == 0) {
            // printf("first token correct : topk_AgeRanges \n");

            char * ktoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");
            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");

            if (ktoken == NULL) {
                printf("invalid k \n");
                 _errors_aggregator++;
            } else if (countrytoken == NULL) {
                printf("invalid countrytoken \n");
                 _errors_aggregator++;
            } else if (diseasetoken == NULL) {
                printf("invalid disease \n");
                 _errors_aggregator++;
            } else if (secondtoken == NULL) {
                printf("Invalid date \n");
                 _errors_aggregator++;
            } else if (thirdtoken == NULL) {
                printf("Invalid date \n");
                 _errors_aggregator++;
            } else {
                //    char * dateA = secondtoken;
                //    char * dateB = thirdtoken;

                topKAgeRanges(ktoken, countrytoken, diseasetoken, secondtoken, thirdtoken);
            }
        } else if (strcmp(firsttoken, "numPatientAdmissions") == 0 || strcmp(firsttoken, "npa") == 0) {
            // printf("first token correct : numPatientAdmissions \n");

            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");

            if (diseasetoken == NULL) {
                printf("invalid disease \n");
                 _errors_aggregator++;
            }
            if (secondtoken == NULL) {
                printf("Invalid date \n");
                 _errors_aggregator++;
            } else {
                char * dateA = secondtoken;
                if (dateA != NULL) {

                    char * dateB = thirdtoken;
                    if (dateB == NULL) {
                        printf("third token invalid \n");
                         _errors_aggregator++;
                    } else if (countrytoken == NULL) {
                        // for (i = 0; i < _total_countries_aggregator; i++) {
                        PatientAdmissionsWithoutCountry(diseasetoken, dateA, dateB);
                        // }
                    } else {
                        PatientAdmissionsWithCountry(diseasetoken, dateA, dateB, countrytoken);
                    }
                }
            }
        } else if (strcmp(firsttoken, "numPatientDischarges") == 0 || strcmp(firsttoken, "npd") == 0) {
            //  printf("first token correct : numPatientDischarges \n");

            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");

            if (diseasetoken == NULL) {
                printf("invalid disease \n");
                 _errors_aggregator++;
            }
            if (secondtoken == NULL) {
                printf("Invalid date \n");
                 _errors_aggregator++;
            } else {
                char * dateA = secondtoken;
                if (dateA != NULL) {

                    char * dateB = thirdtoken;
                    if (dateB == NULL) {
                        printf("third token invalid \n");
                         _errors_aggregator++;
                    } else if (countrytoken == NULL) {
                        //for (i = 0; i < _total_countries_aggregator; i++) {
                        PatientDisChargesWithoutCountry(diseasetoken, dateA, dateB);
                        // }
                    } else {
                        PatientDisChargesWithCountry(diseasetoken, dateA, dateB, countrytoken);
                    }
                }
            }
        }

        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }
    }

}
//// --------------------------------------------------------------------------