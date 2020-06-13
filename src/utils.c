/*
* Copyright (c) 2018 tatata1985
* All rights reserved.
*/
/*
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
*/

#include <sys/types.h> /* struct stat */
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h> /* open */
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h> /* map */
#include <stdio.h> /* printf */

#include <errno.h> /* error */
#include <string.h> /* strerror */
#include "utils.h"

char *attachmemory(char *filepath, size_t *size)
{
	char *mmapedbuff = NULL;
	int fd;
	struct stat st;
	if (stat(filepath, &st) == -1) {
		return mmapedbuff;
	} else {
		if (size)
			*size = st.st_size;

		if (size && *size == 0)
			goto out;

		if ((fd = open(filepath, O_RDONLY)) == -1) {
			return mmapedbuff;
		}

		mmapedbuff = (char*)mmap(NULL, st.st_size,
					 PROT_READ,
					 MAP_SHARED,
					 fd,
					 0);

		if (mmapedbuff == MAP_FAILED)
			printf("MAP_FAILED %s %s \n",
				   strerror(errno), filepath);
		close(fd);
	}
out:
	return mmapedbuff;
}


