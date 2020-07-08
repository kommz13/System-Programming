#ifndef PATIENTDATA_H
#define PATIENTDATA_H

typedef struct PatientData {
    char * id;
    char * firstname;
    char * lastname;
    char * disease;
    char * country;
    char * date_entry;
    char * date_exit;
    int age;
} PatientData;

void freePatient(PatientData * data);

#endif /* PATIENTDATA_H */

