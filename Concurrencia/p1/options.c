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
	{ .name = "size",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 's'},
	{ .name = "iterations",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 'i'},
	{ .name = "delay",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 'd'},
	{ .name = "help",
	  .has_arg = no_argument,
	  .flag = NULL,
	  .val = 'h'},
	{0, 0, 0, 0}
};

static void usage(int i)
{
	printf(
		"Usage:  array [OPTION]\n"
		"Options:\n"
		"  -t n, --threads=<n>: number of threads\n"
		"  -s n, --size=<n>: array size\n"
		"  -i n, --iterations=<n>: number of iterations\n"
		"  -d n, --delay=<n>: delay between buffer ops\n"
		"  -h, --help: this message\n\n"
	);
	exit(i);
}

static int get_uint(char *arg, int *value)
{
	char *end;
	*value = strtoul(arg, &end, 10);

	return (end != NULL);
}

int handle_options(int argc, char **argv, struct options *opt)
{
	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long (argc, argv, "ht:s:i:d:",
				 long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			if (!get_uint(optarg, &opt->num_threads) || opt->num_threads == 0) {
				printf("'%s': is not an integer > 0\n",
				       optarg);
				usage(-3);
			}
			break;

		case 's':
			if (!get_uint(optarg, &opt->size) || opt->size == 0) {
				printf("'%s': is not an integer > 0\n",
				       optarg);
				usage(-3);
			}
			break;

		case 'i':
			if (!get_uint(optarg, &opt->iterations)) {
				printf("'%s': is not a valid integer\n",
				       optarg);
				usage(-3);
			}
			break;

		case 'd':
			if (!get_uint(optarg, &opt->delay)) {
				printf("'%s': is not a valid integer\n",
				       optarg);
				usage(-3);
			}
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
	return 0;
}

int read_options(int argc, char **argv, struct options *opt) {

	int result = handle_options(argc,argv,opt);

	if (result != 0)
		exit(result);

	if (argc - optind != 0) {
		printf ("Too many arguments\n\n");
		while (optind < argc)
			printf ("'%s' ", argv[optind++]);
		printf ("\n");
		usage(-2);
	}

	return 0;
}
