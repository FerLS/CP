#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "options.h"

#define DELAY_SCALE 1000


struct array {
    int size;
    int *arr;
};


void apply_delay(int delay) {
    for(int i = 0; i < delay * DELAY_SCALE; i++); // waste time
}


int increment(int id, int iterations, int delay, struct array *arr)
{
    int pos, val;

    for(int i = 0; i < iterations; i++) {
        pos = rand() % arr->size;

        printf("%d increasing position %d\n", id, pos);

        val = arr->arr[pos];
        apply_delay(delay);

        val ++;
        apply_delay(delay);

        arr->arr[pos] = val;
        apply_delay(delay);
    }

    return 0;
}


void print_array(struct array arr) {
    int total = 0;

    for(int i = 0; i < arr.size; i++) {
        total += arr.arr[i];
        printf("%d ", arr.arr[i]);
    }

    printf("\nTotal: %d\n", total);
}


int main (int argc, char **argv)
{
    struct options       opt;
    struct array         arr;

    srand(time(NULL));

    // Default values for the options
    opt.num_threads  = 5;
    opt.size         = 10;
    opt.iterations   = 100;
    opt.delay        = 1000;

    read_options(argc, argv, &opt);

    arr.size = opt.size;
    arr.arr  = malloc(arr.size * sizeof(int));

    memset(arr.arr, 0, arr.size * sizeof(int));

    increment(0, opt.iterations, opt.delay, &arr);


    print_array(arr);


    return 0;
}