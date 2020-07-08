#ifndef DATE_H
#define DATE_H


#include <time.h>

// https://www.geeksforgeeks.org/strftime-function-in-c/
// http://man7.org/linux/man-pages/man3/strptime.3.html
// https://www.unix.com/programming/30563-how-compare-dates-c-c.html
// https://codereview.stackexchange.com/questions/33365/heap-implementation-using-pointer
//  https://github.com/robin-thomas/max-heap/blob/master/maxHeap.c

//header error
time_t to_seconds(const char *date);

int date_compare(const char * date1, const char * date2);
#endif /* DATE_H */

