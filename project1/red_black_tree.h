#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H
#include <stdbool.h>
#include <stddef.h>
#include "PatientData.h"
#define BLACK 0
#define RED 1
#define Color unsigned char

typedef struct RbtData {
    PatientData * patientData;
} RbtData;

typedef struct RbtNode {
    struct RbtNode* parent;
    struct RbtNode* left;
    struct RbtNode* right;
    Color color;
    char * key;   // date entry

    RbtData data;
} RbtNode;

RbtNode * initializeRB();

RbtNode* parentRB(RbtNode* n);

RbtNode* leftsiblingRB(RbtNode* n);

RbtNode* rightsiblingRB(RbtNode* n);

RbtNode* grantparentRB(RbtNode* n);

RbtNode* siblingRB(RbtNode* n);

RbtNode* uncleRB(RbtNode* n);

void RotateLeftRB(RbtNode* n);

void RotateRightRB(RbtNode* n);

RbtNode* searchRB(RbtNode * _root, const char * key);

RbtNode* insertRB(RbtNode * _root, PatientData * patient);

void cleanupRBT(RbtNode * _root);

// ----------------- computations

int searchTotalRBT(RbtNode * _root, char * date1, char * date2);

int searchTotalForCountryRBT(RbtNode * _root, char * date1, char * date2, char * country);


#endif

