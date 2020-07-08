#ifndef HD_LISTLIST_H
#define HD_LISTLIST_H
#include "DiseaseData.h"

typedef struct InnerNodeHD {
    struct DiseaseData *diseaseBucket;
    struct InnerNodeHD * next;
} InnerNodeHD;

typedef struct OuterNodeHD {
    int data; // hash value
    struct InnerNodeHD * up;
    struct OuterNodeHD * next;
} OuterNodeHD;


void initializeHD(int size, int bucketcapacity);

void cleanupHD();

OuterNodeHD * insertHD_outer(int hash);

DiseaseData * searchHD(char * id);

InnerNodeHD * insertHD(PatientData * patient);

void searchCurrentPatientsByDisease();

void searchGlobalStatsByDisease();

void searchGlobalStatsByDate(char * date1, char * date2);

#endif

