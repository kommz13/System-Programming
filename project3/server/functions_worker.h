#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "queue.h"


typedef struct  {
    double value;
    char category;
} TopKSort;

int main_worker_from_pthread_create(int id, int workers, Queue * queue);

void initialize(int id, Queue * queue);

void interactWithQueue();

int DiseaseFrequency (char * disease, char * country ,char * date1, char * date2);

int topkAges(char * k, char * country, char * disease, char * date1, char * date2);

int PatientAdmissions(char * disease, char * date1, char * date2, char * country) ;

int PatientDischarges(char * disease, char * date1, char * date2, char * country) ;

void searchPatientRecord(char * id);

#endif