#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <openssl/evp.h>

#include "options.h"
#include "queue.h"
#include "threads.h"

#define MAX_PATH 1024
#define BLOCK_SIZE (10*1024*1024)
#define MAX_LINE_LENGTH (MAX_PATH * 2)

struct file_md5 {
    char *file;
    unsigned char *hash;
    unsigned int hash_size;
};

struct argsThreads{
    char *dir;
    queue *in_q;
    queue *out_q;
    struct options * opt;
    FILE *out;
};

void *get_entries(void *args);

void print_hash(struct file_md5 *md5) {
    for(int i = 0; i < md5->hash_size; i++) {
        printf("%02hhx", md5->hash[i]);
    }
}

void read_hash_file(char *file, char *dir, queue q) {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    char *file_name, *hash;
    int hash_len;

    if((fp = fopen(file, "r")) == NULL) {
        printf("Could not open %s : %s\n", file, strerror(errno));
        exit(0);
    }

    while(fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        char *field_break;
        struct file_md5 *md5 = malloc(sizeof(struct file_md5));

        if((field_break = strstr(line, ": ")) == NULL) {
            printf("Malformed md5 file\n");
            exit(0);
        }
        *field_break = '\0';

        file_name = line;
        hash      = field_break + 2;
        hash_len  = strlen(hash);

        md5->file      = malloc(strlen(file_name) + strlen(dir) + 2);
        sprintf(md5->file, "%s/%s", dir, file_name);
        md5->hash      = malloc(hash_len / 2);
        md5->hash_size = hash_len / 2;

        for(int i = 0; i < hash_len; i+=2)
            sscanf(hash + i, "%02hhx", &md5->hash[i / 2]);

        q_insert(q, md5);
    }
    fclose(fp);
}

void sum_file(struct file_md5 *md5) {
    EVP_MD_CTX *mdctx;
    int nbytes;
    FILE *fp;
    char *buf;

    if((fp = fopen(md5->file, "r")) == NULL) {
        printf("Could not open %s\n", md5->file);
        return;
    }

    buf = malloc(BLOCK_SIZE);
    const EVP_MD *md = EVP_get_digestbyname("md5");

    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);

    while((nbytes = fread(buf, 1, BLOCK_SIZE, fp)) >0)
        EVP_DigestUpdate(mdctx, buf, nbytes);

    md5->hash = malloc(EVP_MAX_MD_SIZE);
    EVP_DigestFinal_ex(mdctx, md5->hash, &md5->hash_size);

    EVP_MD_CTX_destroy(mdctx);
    free(buf);
    fclose(fp);
}

void recurse(char *entry, void *arg) {
    queue q = * (queue *) arg;
    struct stat st;

    stat(entry, &st);

    if(S_ISDIR(st.st_mode)){
        struct argsThreads args;
        args.dir = entry;
        args.in_q = &q;
        get_entries(&args);
    }
}

void add_files(char *entry, void *arg) {
    queue q = * (queue *) arg;
    struct stat st;

    stat(entry, &st);

    if(S_ISREG(st.st_mode)){
        q_insert(q, strdup(entry));
    }
}

void walk_dir(char *dir, void (*action)(char *entry, void *arg), void *arg) {
    DIR *d;
    struct dirent *ent;
    char full_path[MAX_PATH];

    if((d = opendir(dir)) == NULL) {
        printf("Could not open dir %s\n", dir);
        return;
    }

    while((ent = readdir(d)) != NULL) {
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") ==0)
            continue;

        snprintf(full_path, MAX_PATH, "%s/%s", dir, ent->d_name);
        action(full_path, arg);
    }
    closedir(d);
}

void startThreads(thrd_t * threads,int n, void *(*fun)(void *),void * args) {
    for (int i = 0; i < n; ++i) {
        thrd_create(&threads[i], (thrd_start_t) fun,args);
    }
}

void *CalcHash(void * args){

    struct argsThreads *data = (struct argsThreads *) (args);
    struct file_md5 *md5;
    char *ent;

    while((ent = q_remove(*data->in_q)) != NULL ) {         //EJ3

        md5 = malloc(sizeof(struct file_md5));
        md5->file = ent;
        sum_file(md5);
        q_insert(*data->out_q, md5);
    }
    return NULL;
}

void *WriteHash(void *args){

    struct argsThreads *data = (struct argsThreads *) (args);
    struct file_md5 *md5;
    int dirname_len;

    dirname_len = strlen(data->opt->dir) + 1; // length of dir + /

    while((md5 = q_remove(*data->out_q)) != NULL)  {

        fprintf(data->out, "%s: ", md5->file + dirname_len);

        for(int i = 0; i < md5->hash_size; i++)
            fprintf(data->out, "%02hhx", md5->hash[i]);
        fprintf(data->out, "\n");

        free(md5->file);
        free(md5->hash);
        free(md5);
    }
    return NULL;
}

void *ReadHash(void *args){

    struct argsThreads *data = (struct argsThreads *) (args);

    read_hash_file(data->opt->file, data-> opt->dir,*data-> in_q);
    q_setReady(data->in_q,true);

    return NULL;

}

void * CheckHash(void * args){

    struct argsThreads *data = (struct argsThreads *) (args);
    struct file_md5 *md5_in, md5_file;

    while((md5_in = q_remove(*data->in_q))) {
        md5_file.file = md5_in->file;

        sum_file(&md5_file);

        if(memcmp(md5_file.hash, md5_in->hash, md5_file.hash_size)!=0) {
            printf("File %s doesn't match.\nFound:    ", md5_file.file);
            print_hash(&md5_file);
            printf("\nExpected: ");
            print_hash(md5_in);
            printf("\n");
        }

        free(md5_file.hash);

        free(md5_in->file);
        free(md5_in->hash);
        free(md5_in);
    }
    return NULL;
}

void *get_entries(void *args) {

    struct argsThreads *data = (struct argsThreads *) (args);

    walk_dir(data->dir, add_files, data->in_q);
    walk_dir(data->dir  , recurse, data->in_q);
    q_setReady(data->in_q,true);

    return NULL;
}

void check(struct options opt) {
    queue in_q;

    in_q  = q_create(opt.queue_size,"in_Q");

    struct argsThreads args;

    args.opt = &opt;
    args.in_q = &in_q;

    q_setReady(&in_q,false);

    thrd_t readThrd;
    thrd_create(&readThrd, (thrd_start_t) ReadHash, &args);


    thrd_t threadsCheck[opt.num_threads];
    startThreads(threadsCheck,opt.num_threads, CheckHash,&args);

    thrd_join(readThrd,NULL);
    for (int i = 0; i < opt.num_threads; ++i) {
        thrd_join(threadsCheck[i],NULL);
    }
    q_destroy(in_q);
}

void sum(struct options opt) {

    FILE *out = NULL;

    if((out = fopen(opt.file, "w")) == NULL) {
        printf("Could not open output file\n");
        exit(0);
    }
    queue in_q, out_q;

    in_q  = q_create(opt.queue_size,"in_q");
    out_q = q_create(opt.queue_size,"out_q");

    q_setReady(&in_q,false);
    q_setReady(&out_q,false);

    struct argsThreads args;
    args.dir = opt.dir;
    args.in_q = &in_q;
    args.out_q = &out_q;
    args.out = out;
    args.opt = &opt;

    thrd_t entriesThread;
    thrd_create(&entriesThread, (thrd_start_t) get_entries, &args);

    thrd_t calcThreads[opt.num_threads];

    startThreads(calcThreads,opt.num_threads,CalcHash,&args);

    thrd_t writThread;
    thrd_create(&writThread, (thrd_start_t) WriteHash, &args);

    thrd_join(entriesThread,NULL);

    for (int i = 0; i < opt.num_threads; ++i) {
        thrd_join(calcThreads[i], NULL);
    }

    q_setReady(&out_q,true);

    thrd_join(writThread,NULL);

    fclose(out);
    q_destroy(in_q);
    q_destroy(out_q);
}

int main(int argc, char *argv[]) {

    struct options opt;

    opt.num_threads = 5;
    opt.queue_size  = 1000;
    opt.check       = true;
    opt.file        = NULL;
    opt.dir         = NULL;

    read_options (argc, argv, &opt);

    if(opt.check)
        check(opt); // 5 y 6
    else
        sum(opt); //2 ,3 y 4
}