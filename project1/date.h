#ifndef DATE_H
#define DATE_H
#include <time.h>

// https://www.unix.com/programming/30563-how-compare-dates-c-c.html
time_t to_seconds(const char *date);

int date_compare(const char * date1, const char * date2);
#endif