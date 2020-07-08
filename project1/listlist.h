#ifndef LISTLIST_H
#define LISTLIST_H

#include "red_black_tree.h"

typedef struct InnerNode {
    struct PatientData * patientNode;
    struct InnerNode * next;
} InnerNode;

typedef struct OuterNode {
    int data; // hash value
    struct InnerNode * up;
    struct OuterNode * next;
} OuterNode;


void initializeLL(int size);

void cleanupLL();

OuterNode * insertLL_outer(int hash);

PatientData * searchLL(char * id);

void insertLL(PatientData * patient);

void votedPercentLL();

void votePostCodeLL(int pc);

#endif

