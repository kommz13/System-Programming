#ifndef COUNTRYDATA_H
#define COUNTRYDATA_H
#include "red_black_tree.h"

typedef struct CountryData {
    char * name;
    RbtNode * root;
    unsigned total, recovered;
    char * latest_date;
} CountryData;

#endif /* COUNTRYDATA_H */

