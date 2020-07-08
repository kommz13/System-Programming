#include  <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>      /* sockets */
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <netdb.h>          /* gethostbyaddr */
#include <arpa/inet.h> 

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

int _h1 = 0;
int _h2 = 0;
int _h3 = 0;
int _b = 0;
int _total_countries;
char ** _countries;
char inputdir_copy[2000] = "input_dir";
char logfile_name[4000] = "log_file.";

int _successes = 0, _errors = 0;

int _pipeb;
int _up;

int _listeningSocket = -1;
int _port = 0;
char _IPbuffer[20];

void catchinterrupt(int signo) { // SIGINT
    char buffer[4000] = "log_file.";
    char buffer2[4000];
    int i;
    sprintf(buffer2, "%d", getpid());

    strcat(buffer, buffer2);

    FILE * fp = fopen(buffer, "w+");

    for (i = 0; i < _total_countries; i++) {
        fprintf(fp, "%s\n", _countries[i]);
    }

    int total = _successes + _errors;
    fprintf(fp, "Total: %d\n", total);
    fprintf(fp, "Success: %d\n", _successes);
    fprintf(fp, "Error: %d\n", _errors);

    fclose(fp);

    printf("\nlogfile written to disk by worker\n");

    for (i = 0; i < _total_countries; i++) {
        free(_countries[i]);
    }

    free(_countries);

    cleanupLL();

    cleanupHD();

    cleanupHC();

    cleanUpSocket();

    printf("Worker with pid :%d finished .\n", getpid());

    exit(0);
}

void catchinterrupt_2(int signo) {
    loaddir(inputdir_copy);
    printf("\nfiles reloaded by pid: %d (SIGUSR1) \n", getpid());
}

void initialize(int h1, int h2, int h3, int b, int total_countries, char **countries) {
    _h1 = h1;
    _h2 = h2;
    _h3 = h3;
    _b = b;
    _total_countries = total_countries;
    _countries = countries;

    initializeHD(_h1, (_b - sizeof (InnerNodeHD*)) / sizeof (DiseaseData));

    initializeHC(_h2, (_b - sizeof (InnerNodeHC*)) / sizeof (CountryData));

    initializeLL(_h3);


    static struct sigaction act;
    act.sa_handler = catchinterrupt;
    sigfillset(&(act.sa_mask));
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);


    static struct sigaction act2;
    act2.sa_handler = catchinterrupt_2;
    act2.sa_flags = SA_RESTART;
    sigfillset(&(act2.sa_mask));
    sigaction(SIGUSR1, &act2, NULL);


    printf("App initialized \n");
}

void createDataStructures() {
    printf("Structures created \n");
}

int createSocket(char ipbuffer[20]) {
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    socklen_t serverlen = sizeof (server);

    if ((_listeningSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("#### socket");
        exit(1);
    }

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(_port); /* The given port */

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

    _port = ntohs(server.sin_port);

    char hostbuffer[256];

    gethostname(hostbuffer, sizeof (hostbuffer));

    struct hostent *host_entry = gethostbyname(hostbuffer);

    char * tempIPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

    strcpy(_IPbuffer, tempIPbuffer);
    strcpy(ipbuffer, tempIPbuffer);

    return _port;
}

void cleanUpSocket() {
    close(_listeningSocket);
}

void loaddir(char * inputdir) {
    DIR *dir_ptr;
    struct dirent *direntp;
    int i, j;
    char dirname[PATH_MAX];
    char inputfile[PATH_MAX];

    if (inputdir == NULL) {
        return;
    } else {
        strcpy(inputdir_copy, inputdir);
    }
    printf("Input file loaded\n");

    // for each country ...
    for (i = 0; i < _total_countries; i++) {
        char * country = _countries[i];
        sprintf(dirname, "%s/%s", inputdir, country);

//        printf("importing from country: %s from dir: %s\n", country, dirname);

        if ((dir_ptr = opendir(dirname)) == NULL) {
            fprintf(stderr, "cannot open %s \n", dirname);
            exit(1);
        } else {
            int D = 0;

            while ((direntp = readdir(dir_ptr)) != NULL) {
                if (direntp->d_name[0] == '.') {
                    continue;
                }
                D++;
            }

            closedir(dir_ptr);

            char ** array = malloc(sizeof (char*)*D);

            dir_ptr = opendir(dirname);

            j = 0;
            while ((direntp = readdir(dir_ptr)) != NULL) {
                if (direntp->d_name[0] == '.') {
                    continue;
                }


                array[j++] = strdup(direntp->d_name);
            }

            closedir(dir_ptr);

            quicksort_entry(array, D, 1);

//            printf("ordered dates: \n");

//            for (j = 0; j < D; j++) {
//                printf("- %s\n", array[j]);
//            }

            for (j = 0; j < D; j++) {
                char * d_name = array[j];

                sprintf(inputfile, "%s/%s/%s", inputdir, country, d_name);

                CountryData * data = searchHC(country);

                if (data != NULL) {
                    if (date_compare(data->latest_date, d_name) == -1) {
                        free(data->latest_date);
                        data->latest_date = strdup(d_name);
                    } else {
                        continue;
                    }
                }


                FILE * fp = fopen(inputfile, "r");

                int N = 0;

                char buffer[1000];

                while (fgets(buffer, sizeof (buffer), fp) != NULL) {
                    N++;
                }

                fclose(fp);

//                printf("Importing file: %s (lines: %d) \n", inputfile, N);

                fp = fopen(inputfile, "r");

                while (fgets(buffer, sizeof (buffer), fp) != NULL) {
                    char * firsttoken = strtok(buffer, " \n");
                    char * enter_exit = strtok(NULL, " \n");

                    char * a = strtok(NULL, " \n");
                    char * b = strtok(NULL, " \n");
                    char * c = strtok(NULL, " \n");
                    char * d = strtok(NULL, " \n");

                    char *copyid = strdup(firsttoken);
                    char *copyfirstname = strdup(a);
                    char *copylastname = strdup(b);
                    char *copydisease = strdup(c);
                    int age = atoi(d);
                    char *copycountry = strdup(country);

                    if (age < 0 || age > 120) {
                        printf("invalid age at line: %s \n", buffer);
                        exit(1); // or continue
                    }

                    if (strcmp(enter_exit, "ENTER") == 0) {
                        char *copydate_entry = strdup(d_name);
                        char * copydate_exit = NULL;


                        PatientData * patient = malloc(sizeof (PatientData));
                        patient->id = copyid;
                        patient->firstname = copyfirstname;
                        patient->lastname = copylastname;
                        patient->disease = copydisease;
                        patient->country = copycountry;
                        patient->date_entry = copydate_entry;
                        patient->date_exit = copydate_exit;
                        patient->age = age;


                        insertLL(patient);

                        insertHD(patient);

                        insertHC(patient);

//                        printf("execution INSERT completed for %s \n", copyid);
                    } else {
                        char *copydate_exit = strdup(d_name);

                        record_patient_exit(copyid, copydate_exit);
                    }

                }

                fclose(fp);
            }

            free(array);
        }

    }
}

void cleanUpDataStructures() {
    catchinterrupt(0);

    printf("Structures cleaned\n");

    cleanupLL();

    cleanupHD();

    cleanupHC();
}

void ins_patient_record(char * id, char * firstname, char * lastname, char * disease, char * country, char * date_entry, char * date_exit) {
//    printf("executing function ins_patient_record for: %s %s %s %s %s %s %s\n", id, firstname, lastname, disease, country, date_entry, date_exit);

    char *copyid = strdup(id);
    char *copyfirstname = strdup(firstname);
    char *copylastname = strdup(lastname);
    char *copydisease = strdup(disease);
    char *copycountry = strdup(country);
    char *copydate_entry = strdup(date_entry);
    char *copydate_exit = NULL;

    if (date_exit != NULL) {
        copydate_exit = strdup(date_exit);
    }

    PatientData * patient = malloc(sizeof (PatientData));
    patient->id = copyid;
    patient->firstname = copyfirstname;
    patient->lastname = copylastname;
    patient->disease = copydisease;
    patient->country = copycountry;
    patient->date_entry = copydate_entry;
    patient->date_exit = copydate_exit;

    if (searchLL(patient->id) == NULL) {
        insertLL(patient);

        insertHD(patient);

        insertHC(patient);

        printf("execution completed for %s \n", copyid);
    } else {
        freePatient(patient);
        printf("execution failed \n");
    }
}

void record_patient_exit(char * id, char * date_exit) {
//    printf("executing function record_patient_exit for id: %s with date %s \n", id, date_exit);

    PatientData * i;
    if ((i = searchLL(id)) == NULL) {
        printf("execution failed, patient not found \n");
    } else {
        if (i->date_exit != NULL) {
            printf("execution failed, patient has already recovered \n");
        } else {
            if (date_compare(i->date_entry, date_exit) <= 0) {
                i->date_exit = strdup(date_exit);

                DiseaseData * data = searchHD(i->disease);
                data->recovered++;
            } else {
                printf("execution failed, date is not valid\n");
            }
        }
    }
}

int num_Current_Patients(char * disease) {
    printf("Executing function num_Current_Patients for disease %s \n", disease);

    DiseaseData * data;

    if (disease != NULL) {
        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
        } else {
            int patients = data->total - data->recovered;
            printf("%d \n", patients);
        }
    } else {
        searchCurrentPatientsByDisease();
    }

    return 0;
}

int globalDiseaseStatsNoDate() {
    printf("Executing function GlobalDiseaseSats \n");

    searchGlobalStatsByDisease();

    return 0;
}

int globalDiseaseStats(char * date1, char * date2) {
    printf("Executing function GlobalDiseaseSats for dates : %s---%s \n", date1, date2);

    searchGlobalStatsByDate(date1, date2);
    return 0;
}

int topkDiseases(char * number, char * country, char * date1, char * date2) {
    printf("Executing function topkDiseases for country %s between dates %s and %s \n", country, date1, date2);
    return 0;
}

int topKCountries(char * number, char * disease, char * date1, char * date2) {
    printf("Executing function topkCountries for disease %s between dates %s and %s \n", disease, date1, date2);
    return 0;
}

void sendCountriesToServer(int socket_b, char * serverip, int serverport, char * ip, int port) {
    int sock, i;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*) &server;
    struct hostent rem = { 0 };

    char * buffer = calloc(1, (sizeof (char)*_pipeb));

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memcpy(&rem, gethostbyname(serverip), sizeof(struct hostent));
     
    server.sin_family = AF_INET; /* Internet domain */
    memcpy(&server.sin_addr, rem.h_addr, rem.h_length);
    server.sin_port = htons(serverport); /* Server port */

    if (connect(sock, serverptr, sizeof (server)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Connected to server: to %s port %d\n", serverip, serverport);

    Response response = {0};

    response.status = 1; // I am a worker
    response.number = socket_b;

    writeall(sock, &response, sizeof (response));

    sprintf(response.buffer, "%s", ip); // IP
    memcpy(buffer, &response, sizeof (Response));
    writeall(sock, buffer, _pipeb);


    sprintf(response.buffer, "%d", port); // PORT
    memcpy(buffer, &response, sizeof (Response));
    writeall(sock, buffer, _pipeb);

    for (i = 0; i < _total_countries; i++) {
        sprintf(response.buffer, "%s", _countries[i]); // PORT
        memcpy(buffer, &response, sizeof (Response));
        writeall(sock, buffer, _pipeb);
    }



    close(sock);

    free(buffer);
}

void interactWithSocket(int pipe_b, int down, int up) {
    struct sockaddr_in client;
    socklen_t clientlen = sizeof (client);
    struct sockaddr *clientptr = (struct sockaddr *) &client;

    char buffer[5000];
    int i;

    _pipeb = pipe_b;
    _up = up;

    Request request;

    sigset_t set1;

    sigfillset(&set1);


    if (_listeningSocket != -1) {
        close(_up);
    }

    while (1) {
        char * pipe_buffer = calloc(1, sizeof (char)*pipe_b);

        memset(pipe_buffer, 0, pipe_b);

        sigprocmask(SIG_UNBLOCK, &set1, NULL);

        if (_listeningSocket == -1) {
            int n = readall(down, pipe_buffer, pipe_b);

            if (n == -1) {
                continue;
            }

            if (n == 0) {
                break;
            }
        } else {
            int newsock;

            printf("Waiting for server to connect ... \n");
            
            if ((newsock = accept(_listeningSocket, clientptr, &clientlen)) < 0) {
                perror("accept");
                exit(1);
            }
            
            printf("Server connected (pipe_b = %d ) \n", pipe_b);

            int n = readall(newsock, pipe_buffer, pipe_b);
            
            _up = newsock;           

            if (n == -1) {
                continue;
            }

            if (n == 0) {
                break;
            }
        }

        sigprocmask(SIG_BLOCK, &set1, NULL);


        memcpy(&request, pipe_buffer, sizeof (Request));

        free(pipe_buffer);

        sprintf(buffer, "%s %s %s %s %s %s",
                request.cmd,
                request.firsttoken,
                request.secondtoken,
                request.thirdtoken,
                request.fourthtoken,
                request.fifthtoken);

         printf("worker incoming buffer: %s \n", buffer);

        char * firsttoken = strtok(buffer, " ");

        if (strcmp(firsttoken, "recordPatientExit") == 0 || strcmp(firsttoken, "rpe") == 0) {
            //printf("first token correct: recordPatientExit \n");

            char * secondtoken = strtok(NULL, " ");

            if (secondtoken == NULL) {
                printf("second token invalid \n");
            } else {
                char * tokenA = secondtoken;
                char * tokenB = strtok(NULL, " ");

                if (tokenA != NULL && tokenB != NULL) {
                    record_patient_exit(tokenA, tokenB);
                }
            }
        } else if (strcmp(firsttoken, "sync") == 0) {
            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            Response response = {0};
            strcpy(response.buffer, "ready");
            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);
        } else if (strcmp(firsttoken, "searchPatientRecord") == 0 || strcmp(firsttoken, "sp") == 0) {
            char * secondtoken = strtok(NULL, " ");

            if (secondtoken == NULL) {
                printf("second token invalid \n");
                _errors++;
            } else {
                char * tokenA = secondtoken;

                if (tokenA != NULL) {
                    searchPatientRecord(tokenA);
                }
            }

        } else if (strcmp(firsttoken, "listCountries") == 0 || strcmp(firsttoken, "li") == 0) {
            for (i = 0; i < _total_countries; i++) {
                printf("%s ", _countries[i]);
                _successes++;
            }
            printf("\n");
        } else if (strcmp(firsttoken, "insertPatientRecord") == 0 || strcmp(firsttoken, "ins") == 0) {
            //printf("first token correct: insertPatientRecord \n");

            char * secondtoken = strtok(NULL, " ");

            if (secondtoken == NULL) {
                printf("second token invalid \n");
                _errors++;
            } else {
                char * tokenA = secondtoken;
                char * tokenB = strtok(NULL, " ");
                char * tokenC = strtok(NULL, " ");
                char * tokenD = strtok(NULL, " ");
                char * tokenE = strtok(NULL, " ");
                char * tokenF = strtok(NULL, " ");
                char * tokenG = strtok(NULL, " ");

                if (tokenA != NULL && tokenB != NULL && tokenC != NULL && tokenD != NULL && tokenE != NULL && tokenF != NULL && tokenG != NULL) {
                    ins_patient_record(tokenA, tokenB, tokenC, tokenD, tokenE, tokenF, tokenG);
                }

                if (tokenA != NULL && tokenB != NULL && tokenC != NULL && tokenD != NULL && tokenE != NULL && tokenF != NULL && tokenG == NULL) {
                    ins_patient_record(tokenA, tokenB, tokenC, tokenD, tokenE, tokenF, NULL);
                }
            }
        } else if (strcmp(firsttoken, "globalDiseaseStats") == 0 || strcmp(firsttoken, "gds") == 0) {
            printf("first token correct: globalDiseaseStats \n");

            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            if (secondtoken == NULL && thirdtoken == NULL) {
                globalDiseaseStatsNoDate();
            } else {
                char * tokenA = secondtoken;
                if (tokenA != NULL) {
                    char * tokenB = thirdtoken;
                    if (tokenB == NULL) {
                        printf("third token invalid \n");
                    } else
                        globalDiseaseStats(tokenA, tokenB);
                }
            }

        } else if (strcmp(firsttoken, "diseaseFrequency") == 0 || strcmp(firsttoken, "dfreq") == 0) {
            //printf("first token correct : diseaseFrequency \n");

            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");

            if (diseasetoken == NULL) {
                printf("invalid disease \n");
                _errors++;
            }
            if (secondtoken == NULL) {
                printf("Invalid date \n");
                _errors++;
            } else {
                char * dateA = secondtoken;
                if (dateA != NULL) {

                    char * dateB = thirdtoken;
                    if (dateB == NULL) {
                        printf("third token invalid \n");
                        _errors++;
                    } else if (countrytoken == NULL) {
                        DiseaseFrequency(diseasetoken, dateA, dateB, NULL);
                    } else {
                        DiseaseFrequency(diseasetoken, dateA, dateB, countrytoken);
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
                _errors++;
            } else if (countrytoken == NULL) {
                printf("invalid countrytoken \n");
                _errors++;
            } else if (diseasetoken == NULL) {
                printf("invalid disease \n");
                _errors++;
            } else if (secondtoken == NULL) {
                printf("Invalid date \n");
                _errors++;
            } else if (thirdtoken == NULL) {
                printf("Invalid date \n");
                _errors++;
            } else {
                char * dateA = secondtoken;
                char * dateB = thirdtoken;

                topkAges(ktoken, countrytoken, diseasetoken, dateA, dateB);
            }
        } else if (strcmp(firsttoken, "numPatientAdmissions") == 0 || strcmp(firsttoken, "npa") == 0) {
            //printf("first token correct : numPatientAdmissions \n");

            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");

            if (diseasetoken == NULL) {
                printf("invalid disease \n");
                _errors++;
            }
            if (secondtoken == NULL) {
                printf("Invalid date \n");
                _errors++;
            } else {
                char * dateA = secondtoken;
                if (dateA != NULL) {

                    char * dateB = thirdtoken;
                    if (dateB == NULL) {
                        printf("third token invalid \n");
                        _errors++;
                    } else if (countrytoken == NULL) {
                        for (i = 0; i < _total_countries; i++) {
                            PatientAdmissions(diseasetoken, dateA, dateB, _countries[i]);
                        }
                    } else {
                        PatientAdmissions(diseasetoken, dateA, dateB, countrytoken);
                    }
                }
            }
        } else if (strcmp(firsttoken, "numPatientDischarges") == 0 || strcmp(firsttoken, "npd") == 0) {
            //printf("first token correct : numPatientDischarges \n");

            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");

            if (diseasetoken == NULL) {
                printf("invalid disease \n");
                _errors++;
            }
            if (secondtoken == NULL) {
                printf("Invalid date \n");
                _errors++;
            } else {
                char * dateA = secondtoken;
                if (dateA != NULL) {

                    char * dateB = thirdtoken;
                    if (dateB == NULL) {
                        printf("third token invalid \n");
                        _errors++;
                    } else if (countrytoken == NULL) {
                        for (i = 0; i < _total_countries; i++) {
                            PatientDischarges(diseasetoken, dateA, dateB, _countries[i]);
                        }
                    } else {
                        PatientDischarges(diseasetoken, dateA, dateB, countrytoken);
                    }
                }
            }

        } else if (strcmp(firsttoken, "topk-Diseases") == 0 || strcmp(firsttoken, "topd") == 0) {
            // printf("first token correct : topk-Diseases \n ");

            char * topnumber = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");
            char * dateA = strtok(NULL, " ");
            char * dateB = strtok(NULL, " ");
            if (dateA == NULL && dateB == NULL) {
                topkDiseases(topnumber, countrytoken, NULL, NULL);
            } else {
                char * tokenA = dateA;
                if (tokenA != NULL) {
                    char * tokenB = dateB;
                    if (tokenB == NULL) {
                        printf("third token invalid \n");
                        _errors++;
                    } else
                        topkDiseases(topnumber, countrytoken, tokenA, tokenB);
                }
            }

        } else if (strcmp(firsttoken, "topk-Countries") == 0 || strcmp(firsttoken, "topc") == 0) {
            // printf("first token correct : topk-Countries \n");

            char * topnumber = strtok(NULL, " ");
            char * diseasetoken = strtok(NULL, " ");
            char * dateA = strtok(NULL, " ");
            char * dateB = strtok(NULL, " ");
            if (dateA == NULL && dateB == NULL) {
                topKCountries(topnumber, diseasetoken, NULL, NULL);
            } else {
                char * tokenA = dateA;
                if (tokenA != NULL) {
                    char * tokenB = dateB;
                    if (tokenB == NULL) {
                        printf("third token invalid \n");
                        _errors++;
                    } else
                        topKCountries(topnumber, diseasetoken, tokenA, tokenB);
                }
            }
        } else if (strcmp(firsttoken, "numCurrentPatients") == 0 || strcmp(firsttoken, "ncp") == 0) {
            //printf("first token correct : numCurrentPatients \n");

            char * diseasetoken = strtok(NULL, " ");
            if (diseasetoken == NULL) {
                num_Current_Patients(NULL);
            } else
                num_Current_Patients(diseasetoken);
        }
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        if (_listeningSocket != -1) {
            close(_up);
        }
    }
}

// --------------------------------------------------------------------------

int DiseaseFrequency(char * disease, char * date1, char * date2, char * country) {
    printf("Executing Function DiseaseFrequency for disease %s in country %s between date %s and %s \n", disease, country, date1, date2);

    if (country == NULL) {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
            _errors++;

            Response response = {0};
            response.status = 0;

            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);

        } else {
            RbtNode * root = data->root;

            int counter = searchTotalRBT(root, date1, date2);

            Response response = {0};
            response.number = counter;
            response.status = 1;

            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);

            _successes++;
        }
    } else {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
            _errors++;
            Response response = {0};
            response.status = 0;

            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);

            _errors++;
        } else {
            RbtNode * root = data->root;

            int counter = searchTotalForCountryRBT(root, date1, date2, country);
            printf("%d \n", counter);

            Response response = {0};
            response.number = counter;
            response.status = 1;

            char * buffer = calloc(1, sizeof (char)*_pipeb);

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);


            _successes++;
        }
    }
    return 0;
}

int PatientAdmissions(char * disease, char * date1, char * date2, char * country) {
        printf("(Worker) Executing Function PatientAdmissions for disease %s in country %s between date %s and %s \n", disease, country, date1, date2);

    if (country == NULL) {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
            _errors++;
            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            Response response = {0};

            response.number = _total_countries;

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);

        } else {
            RbtNode * root = data->root;
            int i;

            for (i = 0; i < _total_countries; i++) {
                char * country = _countries[i];
                int counter = searchTotalForCountryRBT(root, date1, date2, country);
                Response response = {0};
                response.number = counter;
                strcpy(response.buffer, country);
                response.status = 1;

                char * buffer = calloc(1, (sizeof (char)*_pipeb));

                memcpy(buffer, &response, sizeof (Response));

                writeall(_up, buffer, _pipeb);

                free(buffer);

                _successes++;
            }
        }
    } else {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed at worker, disease not found \n");
            _errors++;
            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            Response response = {0};


            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);
        } else {
            RbtNode * root = data->root;

            int counter = searchTotalForCountryRBT(root, date1, date2, country);
            // printf("WORKER PRINT : %s %d \n", country, counter);

            Response response = {0};
            response.number = counter;
            strcpy(response.buffer, country);
            char * buffer = calloc(1, sizeof (char)*_pipeb);

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);


            _successes++;
        }
    }

    return 0;
}

int PatientDischarges(char * disease, char * date1, char * date2, char * country) {
    if (country == NULL) {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
            _errors++;
            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            Response response = {0};

            response.number = 0;

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);

        } else {
            RbtNode * root = data->root;
            int i;

            for (i = 0; i < _total_countries; i++) {
                char * country = _countries[i];
                int counter = searchTotalDischargesForCountryRBT(root, date1, date2, country);
                Response response = {0};
                response.number = counter;
                strcpy(response.buffer, country);
                response.status = 1;

                char * buffer = calloc(1, (sizeof (char)*_pipeb));

                memcpy(buffer, &response, sizeof (Response));

                writeall(_up, buffer, _pipeb);

                free(buffer);

                _successes++;
            }
        }
    } else {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
            char * buffer = calloc(1, (sizeof (char)*_pipeb));

            Response response = {0};

            response.number = 0;

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);
            _errors++;
        } else {
            RbtNode * root = data->root;

            int counter = searchTotalDischargesForCountryRBT(root, date1, date2, country);
            //printf("%s %d \n", country, counter);

            Response response = {0};
            response.number = counter;
            strcpy(response.buffer, country);

            char * buffer = calloc(1, sizeof (char)*_pipeb);

            memcpy(buffer, &response, sizeof (Response));

            writeall(_up, buffer, _pipeb);

            free(buffer);


            _successes++;
        }
    }

    return 0;
}

int topkAges(char * k, char * country, char * disease, char * date1, char * date2) {
    DiseaseData * data;
    int i;

    if ((data = searchHD(disease)) == NULL) {
        printf("execution of topk failed, disease not found \n");
        _errors++;
        char * buffer = calloc(1, sizeof (char)*_pipeb);

        Response response = {0};
        memcpy(buffer, &response, sizeof (Response));
        writeall(_up, buffer, _pipeb);
        free(buffer);

    } else {
        RbtNode * root = data->root;

        TopKSort * counters = searchAgeStatsForCountryRBT(root, date1, date2, country);

        int j = 3;

        char * buffer = calloc(1, sizeof (char)*_pipeb);

        Response response = {0};

        for (i = 0; i < atoi(k); i++) {
            if (i > 3) {
                _errors++;
                break;
            }

            if (counters[j].category == 'A') {
                sprintf(response.buffer + strlen(response.buffer), "\n --- 0-20: %lf %% \n", counters[j].value);
            }

            if (counters[j].category == 'B') {
                sprintf(response.buffer + strlen(response.buffer), "\n --- 20-40: %lf %% \n", counters[j].value);
            }

            if (counters[j].category == 'C') {
                sprintf(response.buffer + strlen(response.buffer), "\n --- 40-60: %lf %% \n", counters[j].value);
            }

            if (counters[j].category == 'D') {
                sprintf(response.buffer + strlen(response.buffer), "\n --- 60+: %lf %% \n", counters[j].value);
            }

            memcpy(buffer, &response, sizeof (Response));

            _successes++;
            j--;
        }
        writeall(_up, buffer, _pipeb);
        free(buffer);
        free(counters);

    }

    return 0;
}

void searchPatientRecord(char * id) {
     printf("executing function searchPatientRecord for id: %s \n", id);

    char * buffer = calloc(1, sizeof (char)*_pipeb);

    Response response = {0};

    PatientData * i;
    if ((i = searchLL(id)) == NULL) {
        memset(buffer, 0, _pipeb);

        sprintf(response.buffer, "record not found \n");


        memcpy(buffer, &response, sizeof (Response));

        writeall(_up, buffer, _pipeb);

        _errors++;
    } else {

        memset(buffer, 0, _pipeb);

        sprintf(response.buffer, "%s %s %s %s %s %s %s %d\n", i->id, i->firstname, i->lastname, i->disease, i->country, i->date_entry, i->date_exit, i->age);

        memcpy(buffer, &response, sizeof (Response));

        writeall(_up, buffer, _pipeb);


        _successes++;
    }

    free(buffer);
}
