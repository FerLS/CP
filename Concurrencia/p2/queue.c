#include <stdlib.h>
#include <threads.h>
#include <stdbool.h>
#include <stdio.h>
#include "queue.h"

// circular array
typedef struct _queue {
    void **data;
    int size;
    int used;
    int first;

    mtx_t lock;
    cnd_t buffer_empty;
    cnd_t buffer_full;
    bool ready;
    char* name;

} _queue;


queue q_create(int size,char * name) {
    queue q = malloc(sizeof(_queue));

    q->size  = size;
    q->used  = 0;
    q->first = 0;
    q->data  = malloc(size * sizeof(void *));
    q->name = name;

    mtx_init(&q->lock, 0);
    cnd_init(&q->buffer_empty);
    cnd_init(&q->buffer_full);

    return q;
}

int q_elements(queue q) {
    int res = q->used;
    return res;
}

int q_insert(queue q, void *elem) {
    mtx_lock(&q->lock);

    while(q->used == q->size) {
        //printf("%s Espero para insertar\n",q->name);

        if(q->ready) {
            mtx_unlock(&q->lock);
            return 0;
        }
        cnd_wait(&q->buffer_full, &q->lock);
        if(q->ready){
            mtx_unlock(&q->lock);
            return 0 ;
        }
    }
    //printf("%s Inserto\n",q->name);

    q->data[(q->first + q->used) % q->size] = elem;
    q->used++;

    if(q->used == 1){
        cnd_broadcast(&q->buffer_empty);

    }
    mtx_unlock(&q->lock);

    return 0;
}

void *q_remove(queue q) {
    void *res;
    mtx_lock(&q->lock);
    while(q->used == 0 ) {

        //printf( "%s Espero para borrar\n",q->name);
        if(q->ready) {
            mtx_unlock(&q->lock);
            return NULL;
        }
        cnd_wait(&q->buffer_empty, &q->lock);
        if(q->ready){
            mtx_unlock(&q->lock);
            return NULL ;
        }
    }
    //printf("%s Borro\n",q->name);

    res = q->data[q->first];

    q->first = (q->first + 1) % q->size;
    q->used--;

    if(q->used == (q->size -1)){
        cnd_broadcast(&q->buffer_full);

    }
    mtx_unlock(&q->lock);

    return res;
}

void q_destroy(queue q) {

    mtx_destroy(&q->lock);
    cnd_destroy(&q->buffer_empty);
    cnd_destroy(&q->buffer_full);
    free(q->data);
    free(q);
}

void q_setReady(queue *q,bool val){

    (*q)->ready = val;
    if(val){
        cnd_broadcast(&(*q)->buffer_empty);
        cnd_broadcast(&(*q)->buffer_full);
    }
}
