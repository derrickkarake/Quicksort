#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>

static const long Num_To_Sort = 1000000; // testing with smaller number as this was requiring more memory than i could afford


long partition(int *arr, long low, long high) {
    int pivot = arr[high];
    long i = low;
    for (long j = low; j < high; j++) {
        if (arr[j] < pivot) {
            if (i != j) {
               
                int temp = arr[i];      
                arr[i] = arr[j];
                arr[j] = temp;
            }
            i++;
        }
    }
    
    int temp = arr[i];
    arr[i] = arr[high];
    arr[high] = temp;
    return i;
}

void quicksort_s(int *arr, long low, long high) {
    if (low < high) {
        long p = partition(arr, low, high);
        quicksort_s(arr, low, p - 1);
        quicksort_s(arr, p + 1, high);
    }
}

/*quicksort implementation https://www.openmp.org/wp-content/uploads/sc16-openmp-booth-tasking-ruud.pdf
usign the tasking modle because some threads will complete their portions of the sort muchs quicker because of the varying portion sizes
which can be exploited by scheduling new tasks to the threads that have completed their tasks. 
*/
void sort_s(int *arr) {
    quicksort_s(arr, 0, Num_To_Sort - 1);           
}

void quicksort_p(int *arr, long low, long high) {
    if (low < high) {
        long p = partition(arr, low, high);
#pragma omp task                         //Creates a task with its own copy of quicksort_p 
        quicksort_p(arr, low, p - 1);     // for the lest side             
#pragma omp task
        quicksort_p(arr, p + 1, high);  //for the right side 
        
        #pragma omp taskwait                             //Wait until all tasks defined in the current task have completed.
    }
    
}

// Parallel version of your sort
void sort_p(int *arr) {
#pragma omp parallel
    {
#pragma omp single
        quicksort_p(arr, 0, Num_To_Sort - 1);
    }
}

int main() {
    int *arr_s = malloc(sizeof(int) * Num_To_Sort);
    for (long i = 0; i < Num_To_Sort; i++) {
        arr_s[i] = rand();
    }

    // Copy the array so that the sorting function can operate on it directly.
    // Note that this doubles the memory usage.
    // You may wish to test with slightly smaller arrays if you're running out of memory.
    int *arr_p = malloc(sizeof(int) * Num_To_Sort);
    memcpy(arr_p, arr_s, sizeof(int) * Num_To_Sort);

    struct timeval start, end;

    printf("Timing sequential...\n");
    gettimeofday(&start, NULL);
    sort_s(arr_s);
    gettimeofday(&end, NULL);
    double time_s = end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000;
    printf("Took %f seconds\n\n", time_s);

    for (int i = 0; i < Num_To_Sort - 1; i++) {
        if (arr_s[i] > arr_s[i+1]) {
            printf("Sequential sort did not sort numbers!");
            exit(1);
        }
    }

    free(arr_s);

    printf("Timing parallel...\n");
    gettimeofday(&start, NULL);
    sort_p(arr_p);
    gettimeofday(&end, NULL);
    double time_p = end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000;
    printf("Took %f seconds\n\n", time_p);

    for (int i = 0; i < Num_To_Sort - 1; i++) {
        if (arr_p[i] > arr_p[i+1]) {
            printf("Parallel sort did not sort numbers!");
            exit(1);
        }
    }

    free(arr_p);

    printf("Total speedup of x%.4f\n", time_s/time_p);

    return 0;
}
