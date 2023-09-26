#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "options.h"

static struct option long_options[] = {
	{ .name = "threads",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 't'},
	{ .name = "queue_size",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 'q'},
	{ .name = "check",
	  .has_arg = no_argument,
	  .flag = NULL,
	  .val = 'c'},
    { .name = "sum",
      .has_arg = no_argument,
      .val = 's',
      .flag = NULL},
    { .name = "out",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 'o'},
	{ .name = "help",
	  .has_arg = no_argument,
	  .flag = NULL,
	  .val = 'h'},
	{0, 0, 0, 0}
};

static void usage(int i)
{
	printf(
		"Usage:  comp [-s | -c] [OPTIONS] DIR FILE\n"
		"Options:\n"
        "  -c,       --check          check DIR with md5s on FILE\n"
		"  -s,       --sum            compute hashes of DIR an store on FILE\n"
        "  -q n,     --queue_size=n   size of the work queue\n"
		"  -t n,     --threads=n      number of threads\n"
		"  -h,       --help           this message\n\n"
	);
	exit(i);
}

static int get_int(char *arg, int *value)
{
	char *end;
	*value = strtol(arg, &end, 10);

	return (end != NULL);
}

int handle_options(int argc, char **argv, struct options *opt)
{
	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long (argc, argv, "hcsq:t:",
				 long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			if (!get_int(optarg, &opt->num_threads)
			    || opt->num_threads <= 0) {
				printf("'%s': is not a valid integer\n",
				       optarg);
				usage(-3);
			}
			break;

		case 's':
            opt->check = false;
			break;
		case 'q':
			if (!get_int(optarg, &opt->queue_size)
			    || opt->queue_size <= 0) {
				printf("'%s': is not a valid integer\n",
				       optarg);
				usage(-3);
			}
			break;
		case 'c':
            opt->check= true;
			break;
		case '?':
		case 'h':
			usage(0);
			break;

		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
			usage(-1);
		}
	}

    if(optind + 2 > argc) {
        usage(0);
        exit(0);
    } else {
        opt->dir  = argv[optind];
        opt->file = argv[optind + 1];
    }

	return 0;
}

int read_options(int argc, char **argv, struct options *opt) {

	int result = handle_options(argc,argv,opt);

	if (result != 0)
		exit(result);

	if (argc - optind > 2) {
		printf ("Too many arguments\n\n");
		while (optind < argc)
			printf ("'%s' ", argv[optind++]);
		printf ("\n");
		usage(-2);
	}

	return 0;
}
