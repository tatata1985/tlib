/*
* Copyright (c) 2014 tatata1985
* All rights reserved.
*/

#ifndef __csv_s__
#define __csv_s__

#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
// #include "utils_common.h"
/*
how to use sample
if you want to from file.
	////////////////////////////////////////////
	// file読み出し
	////////////////////////////////////////////
	struct csvdata csv;
	csv_init(&csv, 7); // field count
	csv.path = "./files.csv";
	csv_load(&csv);

or memory(buffは編集されます)
	struct csvdata csv;
	csv_init(&csv, 7); // field count
	csv.path = NULL;
	csv.clone = buff;
	csv.size = buff_len;
	csv_load(&csv);

	printf("field:%ld tuples:%ld\n", csv.fields, csv.tuples);

	long f = 0;
	long t = 0;
	for(t = 0;t < csv.tuples;t++) {
		for(f = 0;f <= csv.fields;f++) {
			printf("%s", csv_getter(&csv, t, f));
			if (f != 6)
				printf(",");
		}
		printf("\n");
	}
	csv_detach(&csv);
*/

struct csvdata {
	char *path;
	char *clone;
	int writefd;
	size_t size;
	size_t buffsize;
	size_t fields;
	size_t tuples;
	char **index;
};

void csv_detach(struct csvdata*);

int csv_init(struct csvdata*, size_t);
int csv_get_tuplecount(struct csvdata*);
int csv_insert(struct csvdata*, char*);

char *csv_getter(struct csvdata*, size_t, size_t);
int csv_quicksearch(struct csvdata*, size_t, char*);
char *csv_getter_byname(struct csvdata*, size_t, char*);
void csv_setindex(struct csvdata*);
int csv_conquer(struct csvdata*);
void csv_save(struct csvdata*, char*);
int csv_load(struct csvdata*);
char *csv_decode(char*);

void csv_detach(struct csvdata*);

#endif /* __csv_s__ */



