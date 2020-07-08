#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "red_black_tree.h"
#include "hashdisease.h"
#include "DiseaseData.h"

struct OuterNodeHD * _headHD = NULL;

struct OuterNodeHD ** _hashDiseasePointer = NULL;

int _hashDiseaseSize = 0;
int _diseasebucketcapacity = 0;

unsigned int hashHD(char* str2) {
    unsigned char * str = (unsigned char *) str2;
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != '\0') {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % _hashDiseaseSize;
}

void initializeHD(int size, int diseasebucketcapacity) {
    int i;

    _hashDiseaseSize = size;
    _diseasebucketcapacity = diseasebucketcapacity;

    _hashDiseasePointer = (OuterNodeHD **) malloc(sizeof (OuterNodeHD*) * size);

    for (i = 0; i < size; i++) {
        OuterNodeHD * outer = insertHD_outer(i);
        _hashDiseasePointer[i] = outer;
    }

    printf("Hash disease HD initialized with bucket capacity: %d \n", diseasebucketcapacity);
}

void cleanupHD() {
    int i;
    OuterNodeHD * outerPointer = _headHD;

    if (outerPointer == NULL) {
        return;
    } else {
        while (outerPointer != NULL) {
            OuterNodeHD * temp = outerPointer;
            outerPointer = outerPointer->next;

            InnerNodeHD* innerPointer = temp->up;

            if (innerPointer != NULL) {
                while (innerPointer != NULL) {
                    InnerNodeHD * temp2 = innerPointer;
                    innerPointer = innerPointer->next;

                    for (i = 0; i < _diseasebucketcapacity; i++) {
                        free(temp2->diseaseBucket[i].name);
                        cleanupRBT(temp2->diseaseBucket[i].root);
                    }

                    free(temp2->diseaseBucket);
                    free(temp2);
                }
            }

            free(temp);
        }
    }

    free(_hashDiseasePointer);
}

InnerNodeHD * create_innernodeHD(char * name) {
    int i;
    InnerNodeHD* newnode = (InnerNodeHD *) malloc(sizeof (InnerNodeHD));
    newnode->diseaseBucket = malloc(sizeof (DiseaseData) * _diseasebucketcapacity);
    newnode->next = NULL;

    newnode->diseaseBucket[0].name = strdup(name);
    newnode->diseaseBucket[0].root = initializeRB();
    newnode->diseaseBucket[0].total = 0;
    newnode->diseaseBucket[0].recovered = 0;

    for (i = 1; i < _diseasebucketcapacity; i++) {
        newnode->diseaseBucket[i].name = NULL;
        newnode->diseaseBucket[i].root = NULL;
        newnode->diseaseBucket[i].total = 0;
        newnode->diseaseBucket[i].recovered = 0;
    }
    return newnode;
}

OuterNodeHD * create_outernodeHD(int hash) {
    OuterNodeHD* newnode = (OuterNodeHD *) malloc(sizeof (OuterNodeHD));
    newnode->up = NULL;
    newnode->next = NULL;
    newnode->data = hash;
    return newnode;
}

OuterNodeHD * insertHD_outer(int hash) {
    OuterNodeHD * horizontalPointer = _headHD;

    if (horizontalPointer == NULL) {
        horizontalPointer = create_outernodeHD(hash);
        _headHD = horizontalPointer;

        return horizontalPointer;
    } else {
        while (horizontalPointer != NULL) {
            if (horizontalPointer->next) {
                horizontalPointer = horizontalPointer->next;
            } else {
                horizontalPointer->next = create_outernodeHD(hash);
                horizontalPointer = horizontalPointer->next;
                return horizontalPointer;
            }
        }
    }
    return NULL;
}

InnerNodeHD * insertHD(PatientData * patient) {
    char * key = patient->disease;

    unsigned int v = hashHD(key);

    int i;

//    printf("insertinbg ... %s \n", patient->id);

    OuterNodeHD * outerNode = _hashDiseasePointer[v];

    InnerNodeHD* innerNode = outerNode->up;

    if (innerNode == NULL) {
        outerNode->up = create_innernodeHD(patient->disease);
        outerNode->up->diseaseBucket[0].root = insertRB(outerNode->up->diseaseBucket[0].root, patient);
        if (patient->date_exit != NULL) {
            outerNode->up->diseaseBucket[0].total++;
            outerNode->up->diseaseBucket[0].recovered++;
        } else {
            outerNode->up->diseaseBucket[0].total++;
        }
    } else {
        InnerNodeHD* innerNode = outerNode->up;

        if (innerNode != NULL) {
            while (innerNode != NULL) {
                for (i = 0; i < _diseasebucketcapacity; i++) {
                    if (innerNode->diseaseBucket[i].name == NULL) {
                        innerNode->diseaseBucket[i].name = strdup(patient->disease);
                        innerNode->diseaseBucket[i].root = initializeRB();
                        if (patient->date_exit != NULL) {
                            innerNode->diseaseBucket[i].total++;
                            innerNode->diseaseBucket[i].recovered++;
                        } else {
                            innerNode->diseaseBucket[i].total++;
                        }
                        return innerNode;
                    }

                    if (strcmp(innerNode->diseaseBucket[i].name, key) == 0) {
                        innerNode->diseaseBucket[i].root = insertRB(innerNode->diseaseBucket[i].root, patient); // collision, leak
                        if (patient->date_exit != NULL) {
                            innerNode->diseaseBucket[i].total++;
                            innerNode->diseaseBucket[i].recovered++;
                        } else {
                            innerNode->diseaseBucket[i].total++;
                        }

                        return innerNode;
                    }

                }

                if (!innerNode->next) {
                    innerNode->next = create_innernodeHD(patient->disease);
                }

                innerNode = innerNode->next;
            }
        }
    }

    return NULL;
}

void searchCurrentPatientsByDisease() {
    OuterNodeHD * outerPointer = _headHD;
    int i;

    if (outerPointer == NULL) {
        printf("No data \n");
        return;
    } else {
        while (outerPointer != NULL) {
            InnerNodeHD* innerNode = outerPointer->up;

            if (innerNode != NULL) {
                while (innerNode != NULL) {
                    for (i = 0; i < _diseasebucketcapacity; i++) {
                        if (innerNode->diseaseBucket[i].name == NULL) {
                            continue;
                        }
                        int patients = innerNode->diseaseBucket[i].total - innerNode->diseaseBucket[i].recovered;
                        printf("Disease: %s  has patients: %d \n", innerNode->diseaseBucket[i].name, patients);
                    }
                    innerNode = innerNode->next;
                }
            }
            outerPointer = outerPointer->next;
        }
    }
}

void searchGlobalStatsByDisease() {
    OuterNodeHD * outerPointer = _headHD;
    int i;

    if (outerPointer == NULL) {
        printf("No data \n");
        return;
    } else {
        while (outerPointer != NULL) {
            InnerNodeHD* innerNode = outerPointer->up;

            if (innerNode != NULL) {
                while (innerNode != NULL) {
                    for (i = 0; i < _diseasebucketcapacity; i++) {
                        if (innerNode->diseaseBucket[i].name == NULL) {
                            continue;
                        }
                        int total = innerNode->diseaseBucket[i].total;
                        printf("Disease: %s  has patients: %d \n", innerNode->diseaseBucket[i].name, total);
                    }
                    innerNode = innerNode->next;
                }
            }
            outerPointer = outerPointer->next;
        }
    }
}

DiseaseData * searchHD(char * id) {
    char * key = id;
    int i;
    unsigned int v = hashHD(key);

    OuterNodeHD * outerNode = _hashDiseasePointer[v];

    InnerNodeHD* innerNode = outerNode->up;

    if (innerNode != NULL) {
        while (innerNode != NULL) {
            for (i = 0; i < _diseasebucketcapacity; i++) {
                if (strcmp(innerNode->diseaseBucket[i].name, id) == 0) {
                    return innerNode->diseaseBucket + i;
                }
            }

            innerNode = innerNode->next;
        }
    }

    return 0;
}

void searchGlobalStatsByDate(char * date1, char * date2) {
    OuterNodeHD * outerPointer = _headHD;
    int i;

    if (outerPointer == NULL) {
        printf("No data \n");
        return;
    } else {
        while (outerPointer != NULL) {
            InnerNodeHD* innerNode = outerPointer->up;

            if (innerNode != NULL) {
                while (innerNode != NULL) {
                    for (i = 0; i < _diseasebucketcapacity; i++) {
                        if (innerNode->diseaseBucket[i].name == NULL) {
                            continue;
                        }
                        RbtNode * root = innerNode->diseaseBucket[i].root;
                        int total = searchTotalRBT(root, date1, date2);
                        printf("Disease: %s  has patients: %d \n", innerNode->diseaseBucket[i].name, total);
                    }
                    innerNode = innerNode->next;
                }
            }
            outerPointer = outerPointer->next;
        }
    }
}