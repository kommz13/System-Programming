#include <string.h>
#include <stdlib.h>
#include "red_black_tree.h"
#include "date.h"
//#include "bloom_filter.h"

RbtNode* create_rbtnode(PatientData * patient) {
    RbtNode* newnode = (RbtNode *) malloc(sizeof (RbtNode));
    newnode->key = strdup(patient->date_entry);
    newnode->data.patientData = patient;
    newnode->left = NULL;
    newnode->parent = NULL;
    newnode->right = NULL;
    newnode->color = BLACK;
    return newnode;
}

RbtNode * initializeRB() {
    RbtNode * _root = NULL;
    return _root;
}

RbtNode* parentRB(RbtNode* n) {
    if (n != NULL) {
        return n->parent;
    } else {
        return NULL;
    }
}

RbtNode* grantparentRB(RbtNode* n) {
    if (n != NULL) {
        return parentRB(parentRB(n));
    } else {
        return NULL;
    }
}

RbtNode* leftsiblingRB(RbtNode* n) {
    if (n != NULL) {
        return n->left;
    } else {
        return NULL;
    }
}

RbtNode* rightsiblingRB(RbtNode* n) {
    if (n != NULL) {
        return n->right;
    } else {
        return NULL;
    }
}

RbtNode* siblingRB(RbtNode* n) {
    if (n != NULL) {
        RbtNode* p = parentRB(n);

        if (p != NULL) {
            if (n == p->left) {
                return p->right;
            } else {
                return p->left;
            }
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

RbtNode* uncleRB(RbtNode* n) {
    return siblingRB(parentRB(n));
}

void RotateLeftRB(RbtNode* n) {
    if (n == NULL) {
        return;
    }

    RbtNode* p = parentRB(n);
    RbtNode* nnew = n->right;

    n->right = nnew->left;
    nnew->left = n;
    n->parent = nnew;

    if (n->right != NULL) {
        n->right->parent = n;
    }

    if (p != NULL) {
        if (n == p->left) {
            p->left = nnew;
        } else if (n == p->right) {
            p->right = nnew;
        }
    }
    nnew->parent = p;
}

void RotateRightRB(RbtNode* n) {
    if (n == NULL) {
        return;
    }
    RbtNode* p = parentRB(n);
    RbtNode* nnew = n->left;

    n->left = nnew->right;
    nnew->right = n;
    n->parent = nnew;

    if (n->left != NULL) {
        n->left->parent = n;
    }

    if (p != NULL) {
        if (n == p->left) {
            p->left = nnew;
        } else if (n == p->right) {
            p->right = nnew;
        }
    }
    nnew->parent = p;
}

RbtNode* searchRB(RbtNode * _root, const char * key) {
    if (_root == NULL) {
        return NULL;
    }

    RbtNode * temp = _root;

    while (temp != NULL) {
        if (strcmp(temp->key, key) == 0) {
            return temp;
        } else if (strcmp(temp->key, key) < 0) {
            temp = leftsiblingRB(temp);
        } else if (strcmp(temp->key, key) > 0) {
            temp = rightsiblingRB(temp);
        }
    }

    return NULL;
}

void RepairRB(RbtNode* n) {
    if (parentRB(n) == NULL) {
        n->color = BLACK;
    } else if (parentRB(n)->color == BLACK) {
        return;
    } else if (uncleRB(n) != NULL && uncleRB(n)->color == RED) {
        parentRB(n)->color = BLACK;
        uncleRB(n)->color = BLACK;
        grantparentRB(n)->color = RED;
        RepairRB(grantparentRB(n));
    } else {
        RbtNode* p1 = parentRB(n);
        RbtNode* g1 = grantparentRB(n);

        if (n == p1->right && p1 == g1->left) {
            RotateLeftRB(p1);
            n = n->left;
        } else if (n == p1->left && p1 == g1->right) {
            RotateRightRB(p1);
            n = n->right;
        }

        RbtNode* p2 = parentRB(n);
        RbtNode* g2 = grantparentRB(n);

        if (n == p2->left) {
            RotateRightRB(g2);
        } else {
            RotateLeftRB(g2);
        }
        p2->color = BLACK;
        g2->color = RED;
    }
}

RbtNode* insertRB(RbtNode * _root, PatientData * patient) {
    if (_root == NULL) {
        _root = create_rbtnode(patient);

        RepairRB(_root);
        return _root;
    }

    RbtNode * temp = _root;
    RbtNode * n = NULL;

    RbtNode * newnode = NULL;

    char * id = patient->date_entry;

    while (temp != NULL) {
        //        if (strcmp(temp->key, id) == 0) {
        //
        //            return NULL;
        //        }
        int comparevalue1 = date_compare(id, temp->key); 
        
        if (comparevalue1 <= 0) {
            if (leftsiblingRB(temp)) {
                temp = leftsiblingRB(temp);
            } else {
                newnode = create_rbtnode(patient);
                temp->left = newnode;
                temp->left->parent = temp;
                n = temp->left;
                break;
            }
        } else {
            if (rightsiblingRB(temp)) {
                temp = rightsiblingRB(temp);
            } else {
                newnode = create_rbtnode(patient);
                temp->right = newnode;
                temp->right->parent = temp;
                n = temp->right;
                break;
            }
        }
    }

    temp = n;

    RepairRB(temp);

    while (parentRB(temp) != NULL) {
        temp = parentRB(temp);
    }
    _root = temp;

    return _root;
}

void cleanupRBTrec(RbtNode * n) {
    if (n != NULL) {
        RbtNode * nl = leftsiblingRB(n);
        RbtNode * nr = rightsiblingRB(n);

        cleanupRBTrec(nl);
        cleanupRBTrec(nr);

        free(n->key);
        free(n);
    }
}

void cleanupRBT(RbtNode * _root) {
    cleanupRBTrec(_root);
}


int _rec_counter;


void searchTotalRBTrec(RbtNode * n, char * date1, char * date2) {
    if (n != NULL) {
        RbtNode * nl = leftsiblingRB(n);
        RbtNode * nr = rightsiblingRB(n);

        char * key = n->key;

        int comparevalue1 = date_compare(key, date1); // key < date1 =>  comparevalue1 = -1                        
        int comparevalue2 = date_compare(key, date2);

        if (comparevalue1 == 0) {
            _rec_counter++;
        } else if (comparevalue2 == 0) {
            _rec_counter++;
        } else if (comparevalue1 == +1 && comparevalue2 == -1) {
            _rec_counter++;
        }

        if (comparevalue1 >= 0) {
            searchTotalRBTrec(nl, date1, date2);
        }

        if (comparevalue2 <= 0) {
            searchTotalRBTrec(nr, date1, date2);
        }
    }
}

int searchTotalRBT(RbtNode * _root, char * date1, char * date2) {
    _rec_counter = 0;

    searchTotalRBTrec(_root, date1, date2);

    return _rec_counter;
}

// ---------------------------------------

void searchTotalForCountryRBTrec(RbtNode * n, char * date1, char * date2, char * country) {
    if (n != NULL) {
        RbtNode * nl = leftsiblingRB(n);
        RbtNode * nr = rightsiblingRB(n);

        char * key = n->key;

        int comparevalue1 = date_compare(key, date1); // key < date1 =>  comparevalue1 = -1                        
        int comparevalue2 = date_compare(key, date2);

        if (strcmp(n->data.patientData->country, country) == 0) {
            if (comparevalue1 == 0) {
                _rec_counter++;
            } else if (comparevalue2 == 0) {
                _rec_counter++;
            } else if (comparevalue1 == +1 && comparevalue2 == -1) {
                _rec_counter++;
            }
        }

        if (comparevalue1 >= 0) {
            searchTotalForCountryRBTrec(nl, date1, date2, country);
        }

        if (comparevalue2 <= 0) {
            searchTotalForCountryRBTrec(nr, date1, date2, country);
        }
    }
}

int searchTotalForCountryRBT(RbtNode * _root, char * date1, char * date2, char * country) {
    _rec_counter = 0;

    searchTotalForCountryRBTrec(_root, date1, date2, country);

    return _rec_counter;
}
