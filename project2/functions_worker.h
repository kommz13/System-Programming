#ifndef FUNCTIONS_H
#define FUNCTIONS_H

typedef struct  {
    double value;
    char category;
} TopKSort;

int main_worker_from_fork(char * inputdir, int pipe_b, int in, int out);

void initialize(int h1, int h2, int h3, int b, int total_countries, char **countries);

void createDataStructures();

void interactWithUser(int pipe_b, int in, int out);

void loaddir(char * filename);

void cleanUpDataStructures();

void ins_patient_record(char * id, char * firstname, char * lastname, char * disease, char * country, char * date_entry, char * date_exit);

void record_patient_exit(char * id, char * date_exit);

int num_Current_Patients (char * disease);

int globalDiseaseStats (char * date1, char * date2);


int topkDiseases (char * number, char * country, char * date1, char * date2);

int topKCountries (char * number, char * disease, char * date1, char * date2);

// 2nd assignment

int DiseaseFrequency (char * disease, char * country ,char * date1, char * date2);

int topkAges(char * k, char * country, char * disease, char * date1, char * date2);

int PatientAdmissions(char * disease, char * date1, char * date2, char * country) ;

int PatientDischarges(char * disease, char * date1, char * date2, char * country) ;

void searchPatientRecord(char * id);
#endif