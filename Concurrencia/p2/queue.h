#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct _queue *queue;

queue q_create(int size,char * name);  // Create a new queue
int   q_elements(queue q);             // number of elements in a queue
int   q_insert(queue q, void *elem); // insert an element into a queue
void *q_remove(queue q);             // remove an element from the queue
void  q_destroy(queue q);             // destroy a queue.
void q_setReady(queue *q,bool val);
#endif
