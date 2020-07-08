#ifndef HC_LISTLIST_H
#define HC_LISTLIST_H
#include "CountryData.h"

typedef struct InnerNodeHC {
    struct CountryData *countryBucket;
    struct InnerNodeHC * next;
} InnerNodeHC;

typedef struct OuterNodeHC {
    int data; // hash value
    struct InnerNodeHC * up;
    struct OuterNodeHC * next;
} OuterNodeHC;


void initializeHC(int size, int bucketcapacity);

void cleanupHC();

OuterNodeHC * insertHC_outer(int hash);

CountryData * searchHC(char * id);

InnerNodeHC * insertHC(PatientData * patient);

void votedPercentHC();

void votePostCodeHC(int pc);

#endif

