#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void initialize(int h1, int h2, int h3, int b);

void createDataStructures();

void interactWithUser();

void loadfile(char * filename);

void cleanUpDataStructures();

void ins_patient_record(char * id, char * firstname, char * lastname, char * disease, char * country, char * date_entry, char * date_exit);

void record_patient_exit(char * id, char * date_exit);

int num_Current_Patients (char * disease);

int globalDiseaseStats (char * date1, char * date2);

int DiseaseFrequency (char * disease, char * country ,char * date1, char * date2);

int topkDiseases (char *, char * country, char * date1, char * date2);

int topKCountries (char *, char * disease, char * date1, char * date2);

#endif