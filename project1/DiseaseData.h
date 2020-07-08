#ifndef DISEASEDATA_H
#define DISEASEDATA_H
#include "red_black_tree.h"

typedef struct DiseaseData{
    char * name;
    RbtNode * root;
    unsigned total, recovered;
} DiseaseData;

#endif

