#include  <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include "functions_worker.h"
#include "CountryData.h"
#include "date.h"
#include "Request.h"
#include "extio.h"
#include "queue.h"
#include "listlist.h"
#include "functions_server.h"
#include  <sys/types.h>
#include <unistd.h>
#include <pthread.h>

int _b = 0;
int _pipeb;
int _up;
int _id;
Queue * _queue;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void initialize(int id, Queue * queue) {
    _id = id;
    _queue = queue;

    printf("Thread initialized \n");
}

void interactWithClient(int _id, int newsockfd, int socket_b) {
    // send socket b
    Response response = {0};

    response.status = 3; // I am a server
    response.number = socket_b;

    writeall(newsockfd, &response, sizeof (response));

    // retrieve command

    char * buffer = calloc(1, sizeof (char)*socket_b);

    memset(buffer, 0, socket_b);

    int n = readall(newsockfd, buffer, socket_b);

    if (n == -1 || n == 0) {
        printf("interactWithClient broke with n = %d \n", n);
        return;
    }

    Request request = {0};

    memcpy(&request, buffer, sizeof (Request));

    printf("command received from client: %s \n", request.cmd);

    char command[5000];
    strcpy(command, request.cmd);

    char * pch = NULL;
    char * firsttoken = strtok_r(command, " ", &pch);

    if (strcmp(firsttoken, "searchPatientRecord") == 0 || strcmp(firsttoken, "sp") == 0) {
        char * secondtoken = strtok_r(NULL, " ", &pch);

        if (secondtoken == NULL) {
            printf("second token invalid \n");
        } else {
            char * tokenA = secondtoken;

            if (tokenA != NULL) {
                requestPatientRecord(request.cmd, &lock, newsockfd, socket_b, tokenA);
            }
        }
    } else if (strcmp(firsttoken, "diseaseFrequency") == 0 || strcmp(firsttoken, "dfreq") == 0) {
        char * diseasetoken = strtok_r(NULL, " ", &pch);
        char * secondtoken = strtok_r(NULL, " ", &pch);
        char * thirdtoken = strtok_r(NULL, " ", &pch);
        char * countrytoken = strtok_r(NULL, " ", &pch);

        if (diseasetoken == NULL) {
            printf("invalid disease \n");
        }
        if (secondtoken == NULL) {
            printf("Invalid date \n");
        } else {
            char * dateA = secondtoken;
            if (dateA != NULL) {

                char * dateB = thirdtoken;
                if (dateB == NULL) {
                    printf("third token invalid \n");
                } else if (countrytoken == NULL) {

                    requestDiseaseFrequencyWithoutCountry(request.cmd, &lock, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken);
                } else {
                    requestDiseaseFrequencyWithCountry(request.cmd, &lock, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken, countrytoken);
                }
            }
        }
    } else if (strcmp(firsttoken, "topk-AgeRanges") == 0 || strcmp(firsttoken, "tar") == 0) {
        char * ktoken = strtok_r(NULL, " ", &pch);
        char * countrytoken = strtok_r(NULL, " ", &pch);
        char * diseasetoken = strtok_r(NULL, " ", &pch);
        char * secondtoken = strtok_r(NULL, " ", &pch);
        char * thirdtoken = strtok_r(NULL, " ", &pch);

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
            //    char * dateA = secondtoken;
            //    char * dateB = thirdtoken;

            topKAgeRanges(request.cmd, &lock, newsockfd, socket_b, ktoken, countrytoken, diseasetoken, secondtoken, thirdtoken);
        }
    } else if (strcmp(firsttoken, "numPatientAdmissions") == 0 || strcmp(firsttoken, "npa") == 0) {
        char * diseasetoken = strtok_r(NULL, " ", &pch);
        char * secondtoken = strtok_r(NULL, " ", &pch);
        char * thirdtoken = strtok_r(NULL, " ", &pch);
        char * countrytoken = strtok_r(NULL, " ", &pch);

        if (diseasetoken == NULL) {
            printf("invalid disease \n");
        }
        if (secondtoken == NULL) {
            printf("Invalid date \n");
        } else {
            char * dateA = secondtoken;
            if (dateA != NULL) {

                char * dateB = thirdtoken;
                if (dateB == NULL) {
                    printf("third token invalid \n");
                } else if (countrytoken == NULL) {
                    patientAdmissionsWithoutCountry(request.cmd, &lock, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken);
                } else {
                    patientAdmissionsWithCountry(request.cmd, &lock, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken, countrytoken);
                }
            }
        }
    } else if (strcmp(firsttoken, "numPatientDischarges") == 0 || strcmp(firsttoken, "npd") == 0) {
        char * diseasetoken = strtok_r(NULL, " ", &pch);
        char * secondtoken = strtok_r(NULL, " ", &pch);
        char * thirdtoken = strtok_r(NULL, " ", &pch);
        char * countrytoken = strtok_r(NULL, " ", &pch);

        if (diseasetoken == NULL) {
            printf("invalid disease \n");
        }
        if (secondtoken == NULL) {
            printf("Invalid date \n");
        } else {
            char * dateA = secondtoken;
            if (dateA != NULL) {

                char * dateB = thirdtoken;
                if (dateB == NULL) {
                    printf("third token invalid \n");
                } else if (countrytoken == NULL) {
                    patientDischargesWithoutCountry(request.cmd, &lock, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken);
                } else {
                    patientDischargesWithCountry(request.cmd, &lock, newsockfd, socket_b, diseasetoken, secondtoken, thirdtoken, countrytoken);
                }
            }
        }
    }

    free(buffer);
}

void interactWithWorker(int _id, int newsockfd, int socket_b) {
    char * pipe_buffer = calloc(1, sizeof (char)*socket_b);
    Response response;
    char ip[50];
    int port;

    memset(pipe_buffer, 0, socket_b);

    printf("Thread %d, Interacting with worker at fd:%d with socket_b:%d \n", _id, newsockfd, socket_b);

    int n = readall(newsockfd, pipe_buffer, socket_b);

    if (n == -1 || n == 0) {
        printf("Thread %d, interactWithClient broke with n = %d \n", _id, n);
        return;
    }

    memcpy(&response, pipe_buffer, sizeof (Response));
    strcpy(ip, response.buffer);


    n = readall(newsockfd, pipe_buffer, socket_b);

    if (n == -1 || n == 0) {
        printf("Thread %d, interactWithClient broke with n = %d \n", _id, n);
        return;
    }

    memcpy(&response, pipe_buffer, sizeof (Response));

    port = atoi(response.buffer);

    printf("Thread %d, fd:%d, received: IP:%s PORT:%d \n", _id, newsockfd, ip, port);

    while (1) {
        n = readall(newsockfd, pipe_buffer, socket_b);

        if (n == -1 || n == 0) {
            break;
        }

        memcpy(&response, pipe_buffer, sizeof (Response));

        CountryData * countrydata = malloc(sizeof (CountryData));
        strcpy(countrydata->name, response.buffer);

        printf("Thread %d, fd:%d, received: IP:%s PORT:%d => %s \n", _id, newsockfd, ip, port, countrydata->name);


        insertLL(ip, port, socket_b, countrydata);
    }


    printf("Thread %d, Finished: servicing worker at fd:%d with socket_b:%d \n", _id, newsockfd, socket_b);

    free(pipe_buffer);
}

void interactWithQueue() {
    //    Request request;

    printf("Worker %d: waiting for order \n", _id);
    while (1) {
        int newsockfd = obtainQ(_queue);

        if (newsockfd <= 0) {
            break;
        }
        printf("Worker %d: processing: %d \n", _id, newsockfd);

        Response res = {0};

        readall(newsockfd, &res, sizeof (res));

        printf("Worker %d: received: %d %d \n", _id, res.number, res.status);

        if (res.status == 1) { // worker
            int socket_b = res.number;
            interactWithWorker(_id, newsockfd, socket_b);
        } else { // client
            int socket_b = getSocketB();
            interactWithClient(_id, newsockfd, socket_b);
        }

        close(newsockfd);


        //        
        //
        //        memcpy(&request, pipe_buffer, sizeof (Request));
        //
        //        free(pipe_buffer);
        //
        //        sprintf(buffer, "%s %s %s %s %s %s",
        //                request.cmd,
        //                request.firsttoken,
        //                request.secondtoken,
        //                request.thirdtoken,
        //                request.fourthtoken,
        //                request.fifthtoken);
        //
        //        // printf("worker incoming buffer: %s \n", buffer);
        //
        //        char * firsttoken = strtok(buffer, " ");
        //
        //        if (strcmp(firsttoken, "recordPatientExit") == 0 || strcmp(firsttoken, "rpe") == 0) {
        //            //printf("first token correct: recordPatientExit \n");
        //
        //            char * secondtoken = strtok(NULL, " ");
        //
        //            if (secondtoken == NULL) {
        //                printf("second token invalid \n");
        //            } else {
        //                char * tokenA = secondtoken;
        //                char * tokenB = strtok(NULL, " ");
        //
        //                if (tokenA != NULL && tokenB != NULL) {
        //                    record_patient_exit(tokenA, tokenB);
        //                }
        //            }
        //        } else if (strcmp(firsttoken, "sync") == 0) {
        //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
        //
        //            Response response = {0};
        //            strcpy(response.buffer, "ready");
        //            memcpy(buffer, &response, sizeof (Response));
        //
        //            writeall(_up, buffer, _pipeb);
        //
        //            free(buffer);
        //        } else if (strcmp(firsttoken, "searchPatientRecord") == 0 || strcmp(firsttoken, "sp") == 0) {
        //            char * secondtoken = strtok(NULL, " ");
        //
        //            if (secondtoken == NULL) {
        //                printf("second token invalid \n");
        //                _errors++;
        //            } else {
        //                char * tokenA = secondtoken;
        //
        //                if (tokenA != NULL) {
        //                    searchPatientRecord(tokenA);
        //                }
        //            }
        //
        //        } else if (strcmp(firsttoken, "listCountries") == 0 || strcmp(firsttoken, "li") == 0) {
        //            for (i = 0; i < _total_countries; i++) {
        //                printf("%s ", _countries[i]);
        //                _successes++;
        //            }
        //            printf("\n");
        //        } else if (strcmp(firsttoken, "insertPatientRecord") == 0 || strcmp(firsttoken, "ins") == 0) {
        //            //printf("first token correct: insertPatientRecord \n");
        //
        //            char * secondtoken = strtok(NULL, " ");
        //
        //            if (secondtoken == NULL) {
        //                printf("second token invalid \n");
        //                _errors++;
        //            } else {
        //                char * tokenA = secondtoken;
        //                char * tokenB = strtok(NULL, " ");
        //                char * tokenC = strtok(NULL, " ");
        //                char * tokenD = strtok(NULL, " ");
        //                char * tokenE = strtok(NULL, " ");
        //                char * tokenF = strtok(NULL, " ");
        //                char * tokenG = strtok(NULL, " ");
        //
        //                if (tokenA != NULL && tokenB != NULL && tokenC != NULL && tokenD != NULL && tokenE != NULL && tokenF != NULL && tokenG != NULL) {
        //                    ins_patient_record(tokenA, tokenB, tokenC, tokenD, tokenE, tokenF, tokenG);
        //                }
        //
        //                if (tokenA != NULL && tokenB != NULL && tokenC != NULL && tokenD != NULL && tokenE != NULL && tokenF != NULL && tokenG == NULL) {
        //                    ins_patient_record(tokenA, tokenB, tokenC, tokenD, tokenE, tokenF, NULL);
        //                }
        //            }
        //        } else if (strcmp(firsttoken, "globalDiseaseStats") == 0 || strcmp(firsttoken, "gds") == 0) {
        //            printf("first token correct: globalDiseaseStats \n");
        //
        //            char * secondtoken = strtok(NULL, " ");
        //            char * thirdtoken = strtok(NULL, " ");
        //            if (secondtoken == NULL && thirdtoken == NULL) {
        //                globalDiseaseStatsNoDate();
        //            } else {
        //                char * tokenA = secondtoken;
        //                if (tokenA != NULL) {
        //                    char * tokenB = thirdtoken;
        //                    if (tokenB == NULL) {
        //                        printf("third token invalid \n");
        //                    } else
        //                        globalDiseaseStats(tokenA, tokenB);
        //                }
        //            }
        //
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
        //                _errors++;
        //            }
        //            if (secondtoken == NULL) {
        //                printf("Invalid date \n");
        //                _errors++;
        //            } else {
        //                char * dateA = secondtoken;
        //                if (dateA != NULL) {
        //
        //                    char * dateB = thirdtoken;
        //                    if (dateB == NULL) {
        //                        printf("third token invalid \n");
        //                        _errors++;
        //                    } else if (countrytoken == NULL) {
        //                        DiseaseFrequency(diseasetoken, dateA, dateB, NULL);
        //                    } else {
        //                        DiseaseFrequency(diseasetoken, dateA, dateB, countrytoken);
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
        //                _errors++;
        //            } else if (countrytoken == NULL) {
        //                printf("invalid countrytoken \n");
        //                _errors++;
        //            } else if (diseasetoken == NULL) {
        //                printf("invalid disease \n");
        //                _errors++;
        //            } else if (secondtoken == NULL) {
        //                printf("Invalid date \n");
        //                _errors++;
        //            } else if (thirdtoken == NULL) {
        //                printf("Invalid date \n");
        //                _errors++;
        //            } else {
        //                char * dateA = secondtoken;
        //                char * dateB = thirdtoken;
        //
        //                topkAges(ktoken, countrytoken, diseasetoken, dateA, dateB);
        //            }
        //        } else if (strcmp(firsttoken, "numPatientAdmissions") == 0 || strcmp(firsttoken, "npa") == 0) {
        //            //printf("first token correct : numPatientAdmissions \n");
        //
        //            char * diseasetoken = strtok(NULL, " ");
        //            char * secondtoken = strtok(NULL, " ");
        //            char * thirdtoken = strtok(NULL, " ");
        //            char * countrytoken = strtok(NULL, " ");
        //
        //            if (diseasetoken == NULL) {
        //                printf("invalid disease \n");
        //                _errors++;
        //            }
        //            if (secondtoken == NULL) {
        //                printf("Invalid date \n");
        //                _errors++;
        //            } else {
        //                char * dateA = secondtoken;
        //                if (dateA != NULL) {
        //
        //                    char * dateB = thirdtoken;
        //                    if (dateB == NULL) {
        //                        printf("third token invalid \n");
        //                        _errors++;
        //                    } else if (countrytoken == NULL) {
        //                        for (i = 0; i < _total_countries; i++) {
        //                            PatientAdmissions(diseasetoken, dateA, dateB, _countries[i]);
        //                        }
        //                    } else {
        //                        PatientAdmissions(diseasetoken, dateA, dateB, countrytoken);
        //                    }
        //                }
        //            }
        //        } else if (strcmp(firsttoken, "numPatientDischarges") == 0 || strcmp(firsttoken, "npd") == 0) {
        //            //printf("first token correct : numPatientDischarges \n");
        //
        //            char * diseasetoken = strtok(NULL, " ");
        //            char * secondtoken = strtok(NULL, " ");
        //            char * thirdtoken = strtok(NULL, " ");
        //            char * countrytoken = strtok(NULL, " ");
        //
        //            if (diseasetoken == NULL) {
        //                printf("invalid disease \n");
        //                _errors++;
        //            }
        //            if (secondtoken == NULL) {
        //                printf("Invalid date \n");
        //                _errors++;
        //            } else {
        //                char * dateA = secondtoken;
        //                if (dateA != NULL) {
        //
        //                    char * dateB = thirdtoken;
        //                    if (dateB == NULL) {
        //                        printf("third token invalid \n");
        //                        _errors++;
        //                    } else if (countrytoken == NULL) {
        //                        for (i = 0; i < _total_countries; i++) {
        //                            PatientDischarges(diseasetoken, dateA, dateB, _countries[i]);
        //                        }
        //                    } else {
        //                        PatientDischarges(diseasetoken, dateA, dateB, countrytoken);
        //                    }
        //                }
        //            }
        //
        //        } else if (strcmp(firsttoken, "topk-Diseases") == 0 || strcmp(firsttoken, "topd") == 0) {
        //            // printf("first token correct : topk-Diseases \n ");
        //
        //            char * topnumber = strtok(NULL, " ");
        //            char * countrytoken = strtok(NULL, " ");
        //            char * dateA = strtok(NULL, " ");
        //            char * dateB = strtok(NULL, " ");
        //            if (dateA == NULL && dateB == NULL) {
        //                topkDiseases(topnumber, countrytoken, NULL, NULL);
        //            } else {
        //                char * tokenA = dateA;
        //                if (tokenA != NULL) {
        //                    char * tokenB = dateB;
        //                    if (tokenB == NULL) {
        //                        printf("third token invalid \n");
        //                        _errors++;
        //                    } else
        //                        topkDiseases(topnumber, countrytoken, tokenA, tokenB);
        //                }
        //            }
        //
        //        } else if (strcmp(firsttoken, "topk-Countries") == 0 || strcmp(firsttoken, "topc") == 0) {
        //            // printf("first token correct : topk-Countries \n");
        //
        //            char * topnumber = strtok(NULL, " ");
        //            char * diseasetoken = strtok(NULL, " ");
        //            char * dateA = strtok(NULL, " ");
        //            char * dateB = strtok(NULL, " ");
        //            if (dateA == NULL && dateB == NULL) {
        //                topKCountries(topnumber, diseasetoken, NULL, NULL);
        //            } else {
        //                char * tokenA = dateA;
        //                if (tokenA != NULL) {
        //                    char * tokenB = dateB;
        //                    if (tokenB == NULL) {
        //                        printf("third token invalid \n");
        //                        _errors++;
        //                    } else
        //                        topKCountries(topnumber, diseasetoken, tokenA, tokenB);
        //                }
        //            }
        //        } else if (strcmp(firsttoken, "numCurrentPatients") == 0 || strcmp(firsttoken, "ncp") == 0) {
        //            //printf("first token correct : numCurrentPatients \n");
        //
        //            char * diseasetoken = strtok(NULL, " ");
        //            if (diseasetoken == NULL) {
        //                num_Current_Patients(NULL);
        //            } else
        //                num_Current_Patients(diseasetoken);
        //        }
        //        if (strncmp(buffer, "exit", 4) == 0) {
        //            break;
        //        }
    }
}

// --------------------------------------------------------------------------

int DiseaseFrequency(char * disease, char * date1, char * date2, char * country) {
    //printf("Executing Function DiseaseFrequency for disease %s in country %s between date %s and %s \n", disease, country, date1, date2);

    //    if (country == NULL) {
    //        DiseaseData * data;
    //
    //        if ((data = searchHD(disease)) == NULL) {
    //            printf("execution failed, disease not found \n");
    //            _errors++;
    //
    //            Response response = { 0 };
    //            response.status = 0;
    //
    //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //        } else {
    //            RbtNode * root = data->root;
    //
    //            int counter = searchTotalRBT(root, date1, date2);
    //
    //            Response response = { 0 };
    //            response.number = counter;
    //            response.status = 1;
    //
    //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //            _successes++;
    //        }
    //    } else {
    //        DiseaseData * data;
    //
    //        if ((data = searchHD(disease)) == NULL) {
    //            printf("execution failed, disease not found \n");
    //            _errors++;
    //            Response response = { 0 };
    //            response.status = 0;
    //
    //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //            _errors++;
    //        } else {
    //            RbtNode * root = data->root;
    //
    //            int counter = searchTotalForCountryRBT(root, date1, date2, country);
    //            printf("%d \n", counter);
    //
    //            Response response = { 0 };
    //            response.number = counter;
    //            response.status = 1;
    //
    //            char * buffer = calloc(1, sizeof (char)*_pipeb);
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //
    //            _successes++;
    //        }
    //    }
    return 0;
}

int PatientAdmissions(char * disease, char * date1, char * date2, char * country) {
    //    printf("(Worker) Executing Function PatientAdmissions for disease %s in country %s between date %s and %s \n", disease, country, date1, date2);

    //    if (country == NULL) {
    //        DiseaseData * data;
    //
    //        if ((data = searchHD(disease)) == NULL) {
    //            printf("execution failed, disease not found \n");
    //            _errors++;
    //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //            Response response = {0};
    //
    //            response.number = _total_countries;
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //        } else {
    //            RbtNode * root = data->root;
    //            int i;
    //
    //            for (i = 0; i < _total_countries; i++) {
    //                char * country = _countries[i];
    //                int counter = searchTotalForCountryRBT(root, date1, date2, country);
    //                Response response = {0};
    //                response.number = counter;
    //                strcpy(response.buffer, country);
    //                response.status = 1;
    //
    //                char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //                memcpy(buffer, &response, sizeof (Response));
    //
    //                writeall(_up, buffer, _pipeb);
    //
    //                free(buffer);
    //
    //                _successes++;
    //            }
    //        }
    //    } else {
    //        DiseaseData * data;
    //
    //        if ((data = searchHD(disease)) == NULL) {
    //            printf("execution failed at worker, disease not found \n");
    //            _errors++;
    //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //            Response response = {0};
    //
    //            
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //        } else {
    //            RbtNode * root = data->root;
    //
    //            int counter = searchTotalForCountryRBT(root, date1, date2, country);
    //            // printf("WORKER PRINT : %s %d \n", country, counter);
    //
    //            Response response = {0};
    //            response.number = counter;
    //            strcpy(response.buffer, country);
    //            char * buffer = calloc(1, sizeof (char)*_pipeb);
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //
    //            _successes++;
    //        }
    //    }

    return 0;
}

int PatientDischarges(char * disease, char * date1, char * date2, char * country) {
    //    if (country == NULL) {
    //        DiseaseData * data;
    //
    //        if ((data = searchHD(disease)) == NULL) {
    //            printf("execution failed, disease not found \n");
    //            _errors++;
    //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //            Response response = {0};
    //
    //            response.number = 0;
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //        } else {
    //            RbtNode * root = data->root;
    //            int i;
    //
    //            for (i = 0; i < _total_countries; i++) {
    //                char * country = _countries[i];
    //                int counter = searchTotalDischargesForCountryRBT(root, date1, date2, country);
    //                Response response = {0};
    //                response.number = counter;
    //                strcpy(response.buffer, country);
    //                response.status = 1;
    //
    //                char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //                memcpy(buffer, &response, sizeof (Response));
    //
    //                writeall(_up, buffer, _pipeb);
    //
    //                free(buffer);
    //
    //                _successes++;
    //            }
    //        }
    //    } else {
    //        DiseaseData * data;
    //
    //        if ((data = searchHD(disease)) == NULL) {
    //            printf("execution failed, disease not found \n");
    //            char * buffer = calloc(1, (sizeof (char)*_pipeb));
    //
    //            Response response = {0};
    //
    //            response.number = 0;
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //            _errors++;
    //        } else {
    //            RbtNode * root = data->root;
    //
    //            int counter = searchTotalDischargesForCountryRBT(root, date1, date2, country);
    //            //printf("%s %d \n", country, counter);
    //
    //            Response response = {0};
    //            response.number = counter;
    //            strcpy(response.buffer, country);
    //
    //            char * buffer = calloc(1, sizeof (char)*_pipeb);
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            writeall(_up, buffer, _pipeb);
    //
    //            free(buffer);
    //
    //
    //            _successes++;
    //        }
    //    }

    return 0;
}

int topkAges(char * k, char * country, char * disease, char * date1, char * date2) {
    //    DiseaseData * data;
    //    int i;
    //
    //    if ((data = searchHD(disease)) == NULL) {
    //        printf("execution of topk failed, disease not found \n");
    //        _errors++;
    //        char * buffer = calloc(1, sizeof (char)*_pipeb);
    //
    //        Response response = {0};
    //        memcpy(buffer, &response, sizeof (Response));
    //        writeall(_up, buffer, _pipeb);
    //        free(buffer);
    //        
    //    } else {
    //        RbtNode * root = data->root;
    //
    //        TopKSort * counters = searchAgeStatsForCountryRBT(root, date1, date2, country);
    //
    //        int j = 3;
    //
    //        char * buffer = calloc(1, sizeof (char)*_pipeb);
    //
    //        Response response = {0};
    //
    //        for (i = 0; i < atoi(k); i++) {
    //            if (i > 3) {
    //                _errors++;
    //                break;
    //            }
    //
    //            if (counters[j].category == 'A') {
    //                sprintf(response.buffer + strlen(response.buffer), "\n --- 0-20: %lf %% \n", counters[j].value);
    //            }
    //
    //            if (counters[j].category == 'B') {
    //                sprintf(response.buffer + strlen(response.buffer), "\n --- 20-40: %lf %% \n", counters[j].value);
    //            }
    //
    //            if (counters[j].category == 'C') {
    //                sprintf(response.buffer + strlen(response.buffer), "\n --- 40-60: %lf %% \n", counters[j].value);
    //            }
    //
    //            if (counters[j].category == 'D') {
    //                sprintf(response.buffer + strlen(response.buffer), "\n --- 60+: %lf %% \n", counters[j].value);
    //            }
    //
    //            memcpy(buffer, &response, sizeof (Response));
    //
    //            _successes++;
    //            j--;
    //        }
    //        writeall(_up, buffer, _pipeb);
    //        free(buffer);
    //        free(counters);
    //
    //    }

    return 0;
}

void searchPatientRecord(char * id) {
    // printf("executing function searchPatientRecord for id: %s \n", id);

    //    char * buffer = calloc(1, sizeof (char)*_pipeb);
    //
    //    Response response = {0};
    //
    //    PatientData * i;
    //    if ((i = searchLL(id)) == NULL) {
    //        memset(buffer, 0, _pipeb);
    //
    //        sprintf(response.buffer, "record not found \n");
    //        
    //
    //        memcpy(buffer, &response, sizeof (Response));
    //
    //        writeall(_up, buffer, _pipeb);
    //
    //        _errors++;
    //    } else {
    //
    //        memset(buffer, 0, _pipeb);
    //
    //        sprintf(response.buffer, "%s %s %s %s %s %s %s %d\n", i->id, i->firstname, i->lastname, i->disease, i->country, i->date_entry, i->date_exit, i->age);
    //
    //        memcpy(buffer, &response, sizeof (Response));
    //
    //        writeall(_up, buffer, _pipeb);
    //
    //
    //        _successes++;
    //    }

    //    free(buffer);
}