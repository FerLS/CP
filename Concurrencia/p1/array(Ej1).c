#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <threads.h>
#include "options.h"

#define DELAY_SCALE 1000


struct array {
    int size;
    int *arr;
};
struct args {

    int iterations;
    int delay;
    struct array *arr;
    mtx_t mutex;
};

struct thread{
    int id;
    struct args *args;
};


void apply_delay(int delay) {
    for (int i = 0; i < delay * DELAY_SCALE; i++); // waste time
}


void *increment(void *args) {
    struct thread *data = (struct thread *) (args);

    int pos, val;
    mtx_lock(&data->args->mutex);
    for (int i = 0; i < data->args->iterations; i++) {
        pos = rand() % data->args->arr->size;

        printf("%d increasing position %d\n", data->id, pos);

        val = data->args->arr->arr[pos];
        apply_delay(data->args->delay);

        val++;
        apply_delay(data->args->delay);

        data->args->arr->arr[pos] = val;
        apply_delay(data->args->delay);
    }
    mtx_unlock(&data->args->mutex);

    free(args);

    return 0;
}


void print_array(struct array arr) {
    int total = 0;

    for (int i = 0; i < arr.size; i++) {
        total += arr.arr[i];
        printf("%d ", arr.arr[i]);
    }

    printf("\nTotal: %d\n", total);
}

void startThreads(thrd_t threads[],int n,struct args * args){

    for (int i = 0; i < n; ++i) {

        struct thread *thread = malloc(sizeof (struct thread));
        thread->args = args;
        thread->id = i;
        thrd_create(&threads[i], (thrd_start_t) increment,thread);

    }
    for (int i = 0; i < n; ++i) {

        thrd_join(threads[i], NULL); // Esperar por thr
    }

}

int main(int argc, char **argv) {
    struct options opt;
    struct array arr;
    struct args args;



    srand(time(NULL));

    // Default values for the options
    opt.num_threads = 5;
    opt.size = 10;
    opt.iterations = 100;
    opt.delay = 1000;

    read_options(argc, argv, &opt);

    arr.size = opt.size;
    arr.arr = malloc(arr.size * sizeof(int));

    memset(arr.arr, 0, arr.size * sizeof(int));

    args.delay = opt.delay;
    args.iterations = opt.iterations;
    args.arr = &arr;



    thrd_t threads[opt.num_threads];

    mtx_init(&args.mutex,0 );
    startThreads(threads,opt.num_threads,&args);
    mtx_destroy(&args.mutex);

    print_array(arr);

    free(arr.arr);

    return 0;
}