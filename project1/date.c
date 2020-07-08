#define _XOPEN_SOURCE

#include "date.h"
#include <time.h>

// https://www.unix.com/programming/30563-how-compare-dates-c-c.html

time_t to_seconds(const char *date) {
    struct tm storage = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    char *p = NULL;
    time_t retval = 0;

    p = (char *) strptime(date, "%d-%m-%Y", &storage);
    if (p == NULL) {
        retval = 0;
    } else {
        retval = mktime(&storage);
    }
    return retval;
}

int date_compare(const char * date1, const char * date2) {
    time_t d1 = to_seconds(date1);
    time_t d2 = to_seconds(date2);

    if (d1 == d2) {
        return 0;
    } else if (d1 < d2) {
        return -1;
    } else {
        return +1;
    }

}
