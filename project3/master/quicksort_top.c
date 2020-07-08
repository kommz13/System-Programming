/* QuickSort */
#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "quicksort_top.h"
#include "date.h"
// swap

void swap_top(TopKSort* a, TopKSort* b) {
    TopKSort temp = *a;
    *a = *b;
    *b = temp;
}

/* This function takes last element as pivot, places 
   the pivot element at its correct position in sorted 
    array, and places all smaller (smaller than pivot) 
   to left of pivot and all greater elements to right 
   of pivot */
int partition_top(TopKSort arr[], int low, int high) {
    TopKSort pivot = arr[high]; // pivot 
    int i = (low - 1); // Index of smaller element 

    for (int j = low; j <= high - 1; j++) {
        // If current element is smaller than the pivot 
        if (arr[j].value < pivot.value) {
            i++;
            swap_top(&arr[i], &arr[j]);
        }

    }
    swap_top(&arr[i + 1], &arr[high]);

    return (i + 1);
}

/* The main function that implements QuickSort 
 arr[] --> Array to be sorted, 
  low  --> Starting index, 
  high  --> Ending index */
void quickSort_top(TopKSort arr[], int low, int high) {
    if (low < high) {
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int pi = partition_top(arr, low, high);

        // Separately sort elements before 
        // partition and after partition 
        quickSort_top(arr, low, pi - 1);
        quickSort_top(arr, pi + 1, high);
    }
}

void quicksort_entry_top(TopKSort * records, int n) {

    quickSort_top(records, 0, n - 1);
}