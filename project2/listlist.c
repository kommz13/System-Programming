#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "red_black_tree.h"
#include "listlist.h"

struct OuterNode * _head = NULL;

struct OuterNode ** _hashIDPointer = NULL;

int _hashIDSize = 0;

unsigned int hashLL(const char* str) {
    int sum = 0;
    int i;
    for (i = 0; i < strlen(str); i++) {
        sum = sum + str[i];
    }

    return sum % _hashIDSize;
}

void initializeLL(int size) {
    int i;

    _hashIDSize = size;

    _hashIDPointer = (OuterNode **) malloc(sizeof (OuterNode*) * size);

    for (i = 0; i < size; i++) {
        OuterNode * outer = insertLL_outer(i);
        _hashIDPointer[i] = outer;
    }
}

void cleanupLL() {

    OuterNode * outerPointer = _head;

    if (outerPointer == NULL) {
        return;
    } else {
        while (outerPointer != NULL) {
            OuterNode * temp = outerPointer;
            outerPointer = outerPointer->next;

            InnerNode* innerPointer = temp->up;

            if (innerPointer != NULL) {
                while (innerPointer != NULL) {
                    InnerNode * temp2 = innerPointer;
                    innerPointer = innerPointer->next;

                    freePatient(temp2->patientNode);
                    free(temp2);
                }
            }

            free(temp);
        }
    }

    free(_hashIDPointer);
}

InnerNode * create_innernode(PatientData * patient) {
    InnerNode* newnode = (InnerNode *) malloc(sizeof (InnerNode));
    newnode->patientNode = patient;
    newnode->next = NULL;
    return newnode;
}

OuterNode * create_outernode(int hash) {
    OuterNode* newnode = (OuterNode *) malloc(sizeof (OuterNode));
    newnode->up = NULL;
    newnode->next = NULL;
    newnode->data = hash;
    return newnode;
}

OuterNode * insertLL_outer(int hash) {
    OuterNode * horizontalPointer = _head;

    if (horizontalPointer == NULL) {
        horizontalPointer = create_outernode(hash);
        _head = horizontalPointer;

        return horizontalPointer;
    } else {
        while (horizontalPointer != NULL) {
            if (horizontalPointer->next) {
                horizontalPointer = horizontalPointer->next;
            } else {
                horizontalPointer->next = create_outernode(hash);
                horizontalPointer = horizontalPointer->next;
                return horizontalPointer;
            }
        }
    }
    return NULL;
}

void insertLL(PatientData * patient) {
    char * key = patient->id;

    unsigned int v = hashLL(key);

    OuterNode * horizontalPointer = _hashIDPointer[v];

    InnerNode* verticalNode = horizontalPointer->up;

    if (verticalNode == NULL) {
        horizontalPointer->up = create_innernode(patient);
    } else {
        InnerNode* n = create_innernode(patient);
        n->next = horizontalPointer->up;
        horizontalPointer->up = n;
    }
}

void insertLL22(RbtNode * rbtNode) {
    //    OuterNode * horizontalPointer = _head;
    //
    //    if (horizontalPointer == NULL) {
    //        horizontalPointer = create_outernode(rbtNode->data.postcode);
    //        _head = horizontalPointer;
    //    } else {
    //        while (horizontalPointer != NULL) {
    //            if (horizontalPointer->data == rbtNode->data.postcode) {
    //                break;
    //            } else {
    //                if (horizontalPointer->next) {
    //                    horizontalPointer = horizontalPointer->next;
    //                } else {
    //                    horizontalPointer->next = create_outernode(rbtNode->data.postcode);
    //                    horizontalPointer = horizontalPointer->next;
    //                    break;
    //                }
    //            }
    //        }
    //    }
    //
    //    InnerNode* verticalNode = horizontalPointer->up;
    //
    //    if (verticalNode == NULL) {
    //        horizontalPointer->up = create_innernode(rbtNode);
    //    } else {
    //        InnerNode* n = create_innernode(rbtNode);
    //        n->next = horizontalPointer->up;
    //        horizontalPointer->up = n;
    //    }
}

void votedPercentLL() {
    //    OuterNode * horizontalPointer = _head;
    //
    //    if (horizontalPointer == NULL) {
    //        return;
    //    } else {
    //        while (horizontalPointer != NULL) {
    //            float x = 0, y = 0;
    //
    //            InnerNode* verticalNode = horizontalPointer->up;
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

void votePostCodeLL(int pc) {
    //    OuterNode * horizontalPointer = _head;
    //
    //    if (horizontalPointer == NULL) {
    //        return;
    //    } else {
    //        while (horizontalPointer != NULL) {
    //            if (horizontalPointer->data == pc) {
    //                int x = 0;
    //
    //                InnerNode* verticalNode = horizontalPointer->up;
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

PatientData * searchLL(char * id) {
    char * key = id;
    unsigned int v = hashLL(key);

    OuterNode * horizontalPointer = _hashIDPointer[v];

    InnerNode* innerNode = horizontalPointer->up;

    if (innerNode != NULL) {
        while (innerNode != NULL) {
            if (strcmp(innerNode->patientNode->id, id) == 0) {
                return innerNode->patientNode;
            }
            innerNode = innerNode->next;
        }
    }

    return 0;
}