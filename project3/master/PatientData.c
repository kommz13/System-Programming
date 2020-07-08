#include <malloc.h>

#include "PatientData.h"

void freePatient(PatientData * data) {
    free(data->id);
    free(data->firstname);
    free(data->lastname);
    free(data->disease);
    free(data->country);
    free(data->date_entry);

    if (data->date_exit != NULL) {
        free(data->date_exit);
    }
    
    free(data);
}