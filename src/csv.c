/*
* Copyright (c) 2014 tatata1985
* All rights reserved.
*/
#include "csv.h"
#include "utils.h"

int csv_init(struct csvdata *csv, size_t field)
{
	memset(csv, 0, sizeof(struct csvdata));
	csv->fields = field;
	return 0;
}

int csv_get_tuplecount(struct csvdata *csv)
{
	return csv->tuples;
}

int csv_insert(struct csvdata *csv, char *linebuff)
{
	if (!linebuff || linebuff[0] == '\0')
		return 0;

	size_t i = 0;
	size_t field = 0;
	if (csv->writefd) {
		if (write(csv->writefd, linebuff, strlen(linebuff)) == -1)
			printf("write error %s L.%d....\n", __func__, __LINE__);
		return strlen(linebuff);
	}

	size_t orglen = strlen(linebuff);
	char *buff = (char*)malloc(orglen + 1);

	memset(buff, 0, orglen + 1);
	memcpy(buff, linebuff, orglen);
	for (;;) {
		if (buff[i] == ',' || buff[i] == '\n') {
			buff[i] = '\0';
			field++;
		} else if (buff[i] == '\"') {
			size_t j = i;
			while (buff[j] == '\"') j++;
			if (i + 2 == j) {
				i = j - 1;
				goto exit_string;			
			}
			i = j;
			for (;;) {
				while (buff[j] != '\"') j++;
				i = j;
				while (buff[j] == '\"') j++;

				if (i != j && (j - i) % 2) {
					i = j - 1;
					goto exit_string;
				}
				i = j;
			}
		}
exit_string:
		if (i >= orglen)
			break;
		i++;
	}

	if (field - 1 != csv->fields) {
		printf("fieldserror field:%ld csv->fields:%ld [%s]\n",
			field,
			csv->fields,
			linebuff);

		goto out;
	}

	if (csv->size) {
		if (csv->size + orglen + 1 > csv->buffsize) {
			csv->buffsize += csv->size + orglen + 1 + 8192;
			csv->clone = (char*)realloc(csv->clone, csv->buffsize);
		}
	} else {
		csv->buffsize = orglen + 8192;
		csv->clone = (char*)malloc(csv->buffsize);
	}
	memcpy(&csv->clone[csv->size], buff, orglen + 1);
	csv->size += orglen;
	csv->tuples++;
out:

	free(buff);
	return 0;
}

char *csv_getter(struct csvdata *csv, size_t tuple, size_t field)
{
	size_t i = 0;
	if (!csv || !csv->clone || tuple > (csv->tuples + 1) ||
	    field > csv->fields)
		return NULL;

	if (csv->index) {
		char *c = csv->index[tuple];
		size_t fieldcount = 0;
		for (;;) {
			if (field == fieldcount)
				return &c[i];

			while (!(c[i] == '\0' || c[i] == -1))
				i++;

			fieldcount++;
			i++;
		}
	}

	size_t offset = ((csv->fields + 1) * tuple) + field;

	if (!offset)
		return csv->clone;

	if (csv->tuples != 0 && csv->tuples == tuple)
		return NULL;

	for(;;) {
		while (csv->clone[i] != '\0')
			i++;

		if (i >= csv->size)
			return NULL;

		offset--;
		if (!offset)
			return &csv->clone[i + 1];
		i++;
	}
	return NULL;
}

int csv_quicksearch(struct csvdata *csv, size_t field, char *key)
{
	if (!csv || !csv->clone || field > csv->fields)
		return -1;

	size_t i = 0;
	size_t count = 0;
	for (;;) {
		while (csv->clone[i] != '\0') i++;
		if (i >= csv->size)
			return -1;

		count++;
		if (!(count % field) && strcmp(key, &csv->clone[i + 1]) == 0)
			return 0;
		i++;
	}
	return -1;
}

char *csv_getter_byname(struct csvdata *csv, size_t tuple, char* fieldname)
{
	if (!csv || !csv->clone || tuple > csv->tuples)
		return NULL;

	size_t i = 0;
	char *p;
	while ((p = csv_getter(csv, 0, i)) != NULL) {
		if (strcmp(p, fieldname) == 0)
			break;
		i++;
	}
	if (strcmp(p, fieldname) != 0)
		return NULL;

	return csv_getter(csv, tuple, i);
}

void csv_setindex(struct csvdata *csv)
{
	if (csv && csv->index)
		return;

	if (csv->size == 0)
		return;

	csv->index = (char**)malloc((csv->tuples + 2) * sizeof(char*));
	if (!csv->index) {
		printf("malloc error.....\n");
		exit(1); /* todo remove here */
	}
	memset(csv->index, 0, csv->tuples * sizeof(char*));

	size_t i = 0;
	size_t j = 0;
	size_t count = 0;

	while (j <= csv->tuples) {
		if (!(count % (csv->fields + 1))) {
			csv->index[j] = &csv->clone[i];
			j++;
		}

		while (i < csv->size) {
			if (csv->clone[i] == '\0')
				break;
			i++;
		}
		count++;
		i++;
		if (i > csv->size)
			goto out;
	}
out:
	return;
}

int csv_conquer(struct csvdata *csv)
{
	size_t i = 0;
	size_t count = 0;
	char *map = NULL;
	if (csv->path != NULL) {
		/* read from file */
		map = attachmemory(csv->path, &csv->size);
		if (csv->size <= 0) {
			printf("mapping error [%s: %dbyte]\n",
				csv->path,
				(int)csv->size);
				
			return -1;
		}

		csv->clone = (char*)malloc(csv->size + 2);
		memset(csv->clone, 0, csv->size + 2);
		memcpy(csv->clone, map, csv->size);

	} else if (csv->clone != NULL && csv->path == NULL) {
		/* read from memory */
	} else {
		if (csv->size == 0)
			return -1;

	}
	csv->fields = 0;
	csv->tuples = 1;

	/* get fields */
	for (;;) {
		if (i >= csv->size)
			break;

		if (csv->clone[i] == '\"' && csv->clone[i + 1] == '{') {

			while (!(csv->clone[i] == '}' && csv->clone[i + 1] == '\"'))
				i++;
			i += 2;
			csv->fields++;
			if (csv->clone[i] == '\n') {
				csv->clone[i] = '\0';
				break;
			}
			csv->clone[i] = '\0';
		} else if (csv->clone[i] == ',') {
			csv->fields++;
			csv->clone[i] = '\0';
		} else if (csv->clone[i] == '\n') {
			csv->fields++;
			csv->clone[i] = '\0';
			break;
		}
		i++;
	}

	csv->fields--;
	i = 0;
	for (;;) {
		count++;
		if (count >= csv->fields)
			break;
		while (csv->clone[i] != '\0')
			i++;
		i++;
	}

	i = 0;
	count = csv->fields;
	for (;;) {
		if (csv->clone[i] == '\0')
			count--;

		i++;
		if (!count)
			break;
	}

	/* get tuples */
	for (;;) {
		i++;
		if (csv->clone[i] == '\0' && csv->clone[i + 1] == '\0')
			break;
	}

	/* parse all elements. */
	i = 0;
	count = csv->fields;
	for (;;) {
		if (csv->clone[i] == '\0')
			count--;
		i++;
		if (!count)
			break;
	}

	while (csv->size > i) {
		if (csv->clone[i] == '\"') {
			i++;
			size_t endable = 0;
notenough:
			while (csv->clone[i] == '\"') {
				endable = !endable;
				i++;
			}

			if (!endable) {
				while (csv->clone[++i] != '\"');
				goto notenough;
			}
		}
		if (csv->clone[i] == '\n')
			csv->tuples++;

		if (csv->clone[i] == '\n' || csv->clone[i] == ',')
			csv->clone[i] = '\0';
		i++;
	}

	if (map)
		munmap(map, csv->size);

	return 1;
}

void csv_save(struct csvdata *csv, char* path)
{
	size_t i = 0;
	size_t j = 0;

	if (csv->writefd || !csv->clone)
		return;

	while (i < csv->size) {
		if (csv->clone[i] == '\0' && j >= csv->fields) {
			if (i + 1 < csv->size)
				csv->clone[i] = '\n';
			j = 0;
		} else if (csv->clone[i] == '\0') {
			csv->clone[i] = ',';
			j++;
		}
		i++;
	}

	int writefd = open(path, O_RDWR | O_CREAT | O_APPEND,
			   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (write(writefd, csv->clone, csv->size - 1) == -1)
		printf("write error %s L.%d....\n", __func__, __LINE__);

	close(writefd);
	return;
}

int csv_load(struct csvdata *csv)
{
	int ret = csv_conquer(csv);
	if (ret < 0)
		return ret;
	csv_setindex(csv);
	return 1;
}

char *csv_decode(char *buff)
{
	return NULL;
}

void csv_detach(struct csvdata* csv)
{
	if (csv->clone && csv->path != NULL) {
		/* read from file */
		free(csv->clone);
		csv->clone = NULL;
	}

	if (csv->index)
		free(csv->index);
		
	csv->index = NULL;
}

