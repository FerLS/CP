#ifndef __OPTIONS_H__
#define __OPTIONS_H__
#include <stdbool.h>

struct options {
    bool check;
    int num_threads;
    int queue_size;
    char *file;
    char *dir;
};

int read_options(int argc, char **argv, struct options *opt);


#endif
