#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "red_black_tree.h"
#include "hashcountry.h"
#include "CountryData.h"

struct OuterNodeHC * _headHC = NULL;

struct OuterNodeHC ** _hashCountryPointer = NULL;

int _hashCountrySize = 0;
int _countrybucketcapacity = 0;

unsigned int hashHC(char *str2) {
    unsigned char * str = (unsigned char *) str2;
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != '\0') {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % _hashCountrySize;
}

void initializeHC(int size, int countrybucketcapacity) {
    int i;

    _hashCountrySize = size;
    _countrybucketcapacity = countrybucketcapacity;

    _hashCountryPointer = (OuterNodeHC **) malloc(sizeof (OuterNodeHC*) * size);

    for (i = 0; i < size; i++) {
        OuterNodeHC * outer = insertHC_outer(i);
        _hashCountryPointer[i] = outer;
    }

    printf("Hash country HC initialized with bucket capacity: %d \n", countrybucketcapacity);
}

void cleanupHC() {
    int i;
    OuterNodeHC * outerPointer = _headHC;

    if (outerPointer == NULL) {
        return;
    } else {
        while (outerPointer != NULL) {
            OuterNodeHC * temp = outerPointer;
            outerPointer = outerPointer->next;

            InnerNodeHC* innerPointer = temp->up;

            if (innerPointer != NULL) {
                while (innerPointer != NULL) {
                    InnerNodeHC * temp2 = innerPointer;
                    innerPointer = innerPointer->next;

                    for (i = 0; i < _countrybucketcapacity; i++) {
                        free(temp2->countryBucket[i].name);
                        cleanupRBT(temp2->countryBucket[i].root);
                    }

                    free(temp2->countryBucket);
                    free(temp2);
                }
            }

            free(temp);
        }
    }

    free(_hashCountryPointer);
}

InnerNodeHC * create_innernodeHC(char * name, char * date) {
    int i;
    InnerNodeHC* newnode = (InnerNodeHC *) malloc(sizeof (InnerNodeHC));
    newnode->countryBucket = malloc(sizeof (CountryData) * _countrybucketcapacity);
    newnode->next = NULL;

    newnode->countryBucket[0].name = strdup(name);
    newnode->countryBucket[0].root = initializeRB();
    newnode->countryBucket[0].total = 0;
    newnode->countryBucket[0].recovered = 0;
    newnode->countryBucket[0].latest_date = strdup(date);

    for (i = 1; i < _countrybucketcapacity; i++) {
        newnode->countryBucket[i].name = NULL;
        newnode->countryBucket[i].root = NULL;
        newnode->countryBucket[i].total = 0;
        newnode->countryBucket[i].recovered = 0;
    }
    return newnode;
}

OuterNodeHC * create_outernodeHC(int hash) {
    OuterNodeHC* newnode = (OuterNodeHC *) malloc(sizeof (OuterNodeHC));
    newnode->up = NULL;
    newnode->next = NULL;
    newnode->data = hash;
    return newnode;
}

OuterNodeHC * insertHC_outer(int hash) {
    OuterNodeHC * horizontalPointer = _headHC;

    if (horizontalPointer == NULL) {
        horizontalPointer = create_outernodeHC(hash);
        _headHC = horizontalPointer;

        return horizontalPointer;
    } else {
        while (horizontalPointer != NULL) {
            if (horizontalPointer->next) {
                horizontalPointer = horizontalPointer->next;
            } else {
                horizontalPointer->next = create_outernodeHC(hash);
                horizontalPointer = horizontalPointer->next;
                return horizontalPointer;
            }
        }
    }
    return NULL;
}

InnerNodeHC * insertHC(PatientData * patient) {
    char * key = patient->country;

    unsigned int v = hashHC(key);

    int i;

    //    printf("inserting ... %s \n", patient->id);

    OuterNodeHC * outerNode = _hashCountryPointer[v];

    InnerNodeHC* innerNode = outerNode->up;

    if (innerNode == NULL) {
        outerNode->up = create_innernodeHC(patient->country, patient->date_entry);
        outerNode->up->countryBucket[0].root = insertRB(outerNode->up->countryBucket[0].root, patient);
        if (patient->date_exit != NULL) {
            outerNode->up->countryBucket[0].total++;
            outerNode->up->countryBucket[0].recovered++;
        } else {
            outerNode->up->countryBucket[0].total++;
        }
    } else {
        InnerNodeHC* innerNode = outerNode->up;

        if (innerNode != NULL) {
            while (innerNode != NULL) {
                for (i = 0; i < _countrybucketcapacity; i++) {
                    if (innerNode->countryBucket[i].name == NULL) {
                        innerNode->countryBucket[i].name = strdup(patient->country);
                        innerNode->countryBucket[i].latest_date = strdup(patient->date_entry);
                        innerNode->countryBucket[i].root = initializeRB();
                        if (patient->date_exit != NULL) {
                            innerNode->countryBucket[i].total++;
                            innerNode->countryBucket[i].recovered++;
                        } else {
                            innerNode->countryBucket[i].total++;
                        }
                        return innerNode;
                    }

                    if (strcmp(innerNode->countryBucket[i].name, key) == 0) {
                        innerNode->countryBucket[i].root = insertRB(innerNode->countryBucket[i].root, patient);
                        free(innerNode->countryBucket[i].latest_date);
                        innerNode->countryBucket[i].latest_date = strdup(patient->date_entry);
                        if (patient->date_exit != NULL) {
                            innerNode->countryBucket[i].total++;
                            innerNode->countryBucket[i].recovered++;
                        } else {
                            innerNode->countryBucket[i].total++;
                        }

                        return innerNode;
                    }

                }

                if (!innerNode->next) {
                    innerNode->next = create_innernodeHC(patient->country, patient->date_entry);
                }

                innerNode = innerNode->next;
            }
        }
    }

    return NULL;
}

void votedPercentHC() {
    //    OuterNodeHC * horizontalPointer = _headHC;
    //
    //    if (horizontalPointer == NULL) {
    //        return;
    //    } else {
    //        while (horizontalPointer != NULL) {
    //            float x = 0, y = 0;
    //
    //            InnerNodeHC* verticalNode = horizontalPointer->up;
    //
    //            if (verticalNode != NULL) {
    //                while (verticalNode != NULL) {
    //                    if (verticalNode->patientNode->voted) {
    //                        x++;
    //                        y++;
    //                    } else {
    //                        y++;
    //                    }
    //
    //                    verticalNode = verticalNode->next;
    //                }
    //            }
    //
    //            float ratio = x / y;
    //
    //            printf("postcode: %d  ratio: %f \n", horizontalPointer->data, ratio * 100);
    //
    //            horizontalPointer = horizontalPointer->next;
    //        }
    //    }
}

void votePostCodeHC(int pc) {
    //    OuterNodeH * horizontalPointer = _headHC;
    //
    //    if (horizontalPointer == NULL) {
    //        return;
    //    } else {
    //        while (horizontalPointer != NULL) {
    //            if (horizontalPointer->data == pc) {
    //                int x = 0;
    //
    //                InnerNodeHC* verticalNode = horizontalPointer->up;
    //
    //                if (verticalNode != NULL) {
    //                    while (verticalNode != NULL) {
    //                        if (verticalNode->patientNode->voted) {
    //                            x++;
    //                        }
    //
    //                        verticalNode = verticalNode->next;
    //                    }
    //                }
    //
    //                printf("IN postcode: %d  VOTERS ARE : %d \n", horizontalPointer->data, x);
    //            }
    //            horizontalPointer = horizontalPointer->next;
    //        }
    //    }
}

CountryData * searchHC(char * id) {
    char * key = id;
    int i;
    unsigned int v = hashHC(key);

    OuterNodeHC * outerNode = _hashCountryPointer[v];

    InnerNodeHC* innerNode = outerNode->up;

    if (innerNode != NULL) {
        while (innerNode != NULL) {
            for (i = 0; i < _countrybucketcapacity; i++) {
                if (strcmp(innerNode->countryBucket[i].name, id) == 0) {
                    return innerNode->countryBucket + i;
                }
            }

            innerNode = innerNode->next;
        }
    }

    return 0;
}