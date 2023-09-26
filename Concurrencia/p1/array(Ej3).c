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

    int lastId;
    int iterations;
    int delay;
    struct array *arr;
    mtx_t *mutex;
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
    for (int i = 0; i < data->args->iterations; i++) {
        pos = rand() % data->args->arr->size;
        mtx_lock(&data->args->mutex[pos]);

        printf("%d increasing position %d\n", data->id, pos);

        val = data->args->arr->arr[pos];
        apply_delay(data->args->delay);

        val++;
        apply_delay(data->args->delay);

        data->args->arr->arr[pos] = val;
        apply_delay(data->args->delay);
        mtx_unlock(&data->args->mutex[pos]);



    }

    free(args);

    return 0;
}

void *incr_decre(void *args) {
    struct thread *data = (struct thread *) (args);

    int pos1, pos2, val1, val2;


    for (int i = 0; i < data->args->iterations; i++) {



        pos1 = rand() % data->args->arr->size;

        do {
            pos2 = rand() % data->args->arr->size;
        } while (pos1 == pos2);

        if (pos1 < pos2) {
            mtx_lock(&data->args->mutex[pos1]);
            mtx_lock(&data->args->mutex[pos2]);
        } else {
            mtx_lock(&data->args->mutex[pos2]);
            mtx_lock(&data->args->mutex[pos1]);
        }


        printf("%d decreasing position %d\n", data->id, pos1);


        val1 = data->args->arr->arr[pos1];
        apply_delay(data->args->delay);

        val1--;
        apply_delay(data->args->delay);

        data->args->arr->arr[pos1] = val1;

        printf("%d increasing position %d\n", data->id, pos2);

        val2 = data->args->arr->arr[pos2];
        apply_delay(data->args->delay);

        val2++;
        apply_delay(data->args->delay);


        data->args->arr->arr[pos2] = val2;

        mtx_unlock(&data->args->mutex[pos2]);
        mtx_unlock(&data->args->mutex[pos1]);

    }
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

void startThreads(thrd_t threads[],int n,struct args * args,void *(*fun)(void*)){

    for (int i = 0; i < n; ++i) {

        int lastId = args->lastId;
        struct thread *thread = malloc(sizeof (struct thread));
        thread->args = args;
        thread->id = lastId;
        thrd_create(&threads[i], (thrd_start_t) fun,thread);
        args->lastId = args->lastId+1;

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
    args.lastId = 0;

    mtx_t mtxs[arr.size];

    args.mutex = mtxs;

    thrd_t threads[opt.num_threads];
    thrd_t extrThreads[opt.num_threads];

    for (int i = 0; i < arr.size; ++i) {
        mtx_init(&mtxs[i],0 );
    }
    startThreads(threads,opt.num_threads,&args,increment);


    startThreads(extrThreads,opt.num_threads,&args,incr_decre);

    for (int i = 0; i < arr.size; ++i) {
        mtx_destroy(&mtxs[i]);
    }
    print_array(arr);

    free(arr.arr);

    return 0;
}