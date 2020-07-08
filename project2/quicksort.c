/* QuickSort */
#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "quicksort.h"
#include "date.h"
// swap

void swap(MyRecord* a, MyRecord* b) {
    MyRecord temp = *a;
    *a = *b;
    *b = temp;
}

/* This function takes last element as pivot, places 
   the pivot element at its correct position in sorted 
    array, and places all smaller (smaller than pivot) 
   to left of pivot and all greater elements to right 
   of pivot */
int partition(MyRecord arr[], int low, int high, int o) {
    MyRecord pivot = arr[high]; // pivot 
    int i = (low - 1); // Index of smaller element 

    for (int j = low; j <= high - 1; j++) {
        // If current element is smaller than the pivot 
        if (o == 1 && date_compare(arr[j], pivot) < 0) {
            i++;
            swap(&arr[i], &arr[j]);
        }
        
        if (o == 2 && strcmp(arr[j], pivot) < 0) {
            i++;
            swap(&arr[i], &arr[j]);
        }

    }
    swap(&arr[i + 1], &arr[high]);

    return (i + 1);
}

/* The main function that implements QuickSort 
 arr[] --> Array to be sorted, 
  low  --> Starting index, 
  high  --> Ending index */
void quickSort(MyRecord arr[], int low, int high, int o ) {
    if (low < high) {
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int pi = partition(arr, low, high, o);

        // Separately sort elements before 
        // partition and after partition 
        quickSort(arr, low, pi - 1, o);
        quickSort(arr, pi + 1, high, o);
    }
}

void quicksort_entry(MyRecord * records, int n, int o) { // o=1:dates o=2 strings

    quickSort(records, 0, n - 1, o);
}