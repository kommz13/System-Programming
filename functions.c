#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "functions.h"
#include "listlist.h"
#include "hashdisease.h"
#include "hashcountry.h"
#include "DiseaseData.h"
#include "CountryData.h"
#include "date.h"

int _h1 = 0;
int _h2 = 0;
int _h3 = 0;
int _b = 0;

void initialize(int h1, int h2, int h3, int b) {
    _h1 = h1;
    _h2 = h2;
    _h3 = h3;
    _b = b;

    //printf("App initialized \n");
}

void createDataStructures() {
    //printf("Structures created \n");
}

void loadfile(char * filename) {
    if (filename == NULL) {
        return;
    }
    //printf("Input file loaded\n");

    FILE * fp = fopen(filename, "r");

    int N = 0;

    char buffer[1000];

    while (fgets(buffer, sizeof (buffer), fp) != NULL) {
        N++;
    }

    fclose(fp);

    _h3 = N / 4;

    initializeLL(_h3);

    initializeHD(_h1, (_b - sizeof (InnerNodeHD*)) / sizeof (DiseaseData));

    initializeHC(_h2, (_b - sizeof (InnerNodeHC*)) / sizeof (CountryData));

    //printf("Lines detected: %d \n", N);
    //printf("H1            : %d \n", _h1);
    //printf("H2            : %d \n", _h2);
    //printf("H3            : %d \n", _h3);

    fp = fopen(filename, "r");

    while (fgets(buffer, sizeof (buffer), fp) != NULL) {
        char * firsttoken = strtok(buffer, " \n");

        char * a = strtok(NULL, " \n");
        char * b = strtok(NULL, " \n");
        char * c = strtok(NULL, " \n");
        char * d = strtok(NULL, " \n");
        char * e = strtok(NULL, " \n");
        char * f = strtok(NULL, " \n");

        char *copyid = strdup(firsttoken);
        char *copyfirstname = strdup(a);
        char *copylastname = strdup(b);
        char *copydisease = strdup(c);
        char *copycountry = strdup(d);
        char *copydate_entry = strdup(e);
        char *copydate_exit ;
        
        if (strcmp(f, "-") == 0) {
            copydate_exit = NULL;
        } else {
            copydate_exit =  strdup(f);
        }

        PatientData * patient = malloc(sizeof (PatientData));
        patient->id = copyid;
        patient->firstname = copyfirstname;
        patient->lastname = copylastname;
        patient->disease = copydisease;
        patient->country = copycountry;
        patient->date_entry = copydate_entry;
        patient->date_exit = copydate_exit;

        insertLL(patient);

        insertHD(patient);

        insertHC(patient);
    }

    fclose(fp);

}

void cleanUpDataStructures() {
    //printf("Structures cleaned\n");

    cleanupLL();

    cleanupHD();

    cleanupHC();
}

void ins_patient_record(char * id, char * firstname, char * lastname, char * disease, char * country, char * date_entry, char * date_exit) {
    printf("/insertPatientRecord\n");

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

        printf("Record added\n");
    } else {
        freePatient(patient);
        printf("execution failed , patient ID already exists\n");
    }
}

void record_patient_exit(char * id, char * date_exit) {
    //printf("/recordPatientExit\n");

    PatientData * i;
    if ((i = searchLL(id)) == NULL) {
        printf("execution failed, patient not found \n");
    } else  if (i->date_exit != NULL) {
            printf("Patient has already recovered \n");
        } else {
            if (date_compare(i->date_entry, date_exit) <= 0) {
                i->date_exit = strdup(date_exit);

                DiseaseData * data = searchHD(i->disease);
                data->recovered++;
                printf("Record updated\n");
            } else {
                printf("execution failed, date is not valid\n");
            }
        }
    
}

int num_Current_Patients(char * disease) {
    printf("/numCurrentPatients\n");

    DiseaseData * data;

    if (disease != NULL) {
        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
        } else {
            int patients = data->total - data->recovered;
            printf("%s %d \n",disease, patients);
        }
    } else {
        searchCurrentPatientsByDisease();
    }

    return 0;
}

int globalDiseaseStatsNoDate() {
    printf("/GlobalDiseaseStats\n");

    searchGlobalStatsByDisease();

    return 0;
}

int globalDiseaseStats(char * date1, char * date2) {
    printf("/GlobalDiseaseSats\n");

    searchGlobalStatsByDate(date1, date2);
    return 0;
}

int DiseaseFrequency(char * disease, char * date1, char * date2, char * country) {
    printf("/diseaseFrequency\n");

    if (country == NULL) {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
        } else {
            RbtNode * root = data->root;

            int counter = searchTotalRBT(root, date1, date2);
            printf("%s %d\n",disease,counter);
        }
    } else {
        DiseaseData * data;

        if ((data = searchHD(disease)) == NULL) {
            printf("execution failed, disease not found \n");
        } else {
            RbtNode * root = data->root;

            int counter = searchTotalForCountryRBT(root, date1, date2, country);
            printf("%s %d\n",disease,counter);
        }
    }
    return 0;
}

int topkDiseases(char * number, char * country, char * date1, char * date2) {
    printf("/topk-Diseases\n");
    return 0;
}

int topKCountries(char * number, char * disease, char * date1, char * date2) {
    printf("/topk-Countries\n");
    return 0;
}

void interactWithUser() {
    char buffer[5000];

    while (1) {
        printf("Type command: ");
        fgets(buffer, sizeof (buffer), stdin);

        int length = strlen(buffer);
        buffer[length - 1] = '\0';

        if (length < 3) {
            printf("unknown first token \n");
            continue;
        }

        char * firsttoken = strtok(buffer, " ");

        if (strcmp(firsttoken, "/recordPatientExit") == 0 || strcmp(firsttoken, "rpe") == 0) {
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
        } else if (strcmp(firsttoken, "/insertPatientRecord") == 0 || strcmp(firsttoken, "ins") == 0) {
            printf("first token correct: insertPatientRecord \n");

            char * secondtoken = strtok(NULL, " ");

            if (secondtoken == NULL) {
                printf("second token invalid \n");
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
        } else if (strcmp(firsttoken, "/globalDiseaseStats") == 0 || strcmp(firsttoken, "gds") == 0) {
            //printf("first token correct: globalDiseaseStats \n");

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

        } else if (strcmp(firsttoken, "/diseaseFrequency") == 0 || strcmp(firsttoken, "dfreq") == 0) {
           // printf("first token correct : diseaseFrequency \n");

            char * diseasetoken = strtok(NULL, " ");
            char * secondtoken = strtok(NULL, " ");
            char * thirdtoken = strtok(NULL, " ");
            char * countrytoken = strtok(NULL, " ");

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
                        DiseaseFrequency(diseasetoken, dateA, dateB, NULL);
                    } else
                        DiseaseFrequency(diseasetoken, dateA, dateB, countrytoken);
                }
            }

        } else if (strcmp(firsttoken, "/topk-Diseases") == 0 || strcmp(firsttoken, "topd") == 0) {
            //printf("first token correct : topk-Diseases \n ");

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
                    } else
                        topkDiseases(topnumber, countrytoken, tokenA, tokenB);
                }
            }

        } else if (strcmp(firsttoken, "/topk-Countries") == 0 || strcmp(firsttoken, "topc") == 0) {
           //printf("first token correct : topk-Countries \n");

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
                    } else
                        topKCountries(topnumber, diseasetoken, tokenA, tokenB);
                }
            }
        } else if (strcmp(firsttoken, "/numCurrentPatients") == 0 || strcmp(firsttoken, "ncp") == 0) {
            //printf("first token correct : numCurrentPatients \n");

            char * diseasetoken = strtok(NULL, " ");
            if (diseasetoken == NULL) {
                num_Current_Patients(NULL);
            } else
                num_Current_Patients(diseasetoken);
        }
        if (strncmp(buffer, "/exit", 5) == 0) {
            printf("exiting\n");
            break;
        }
    }
}
