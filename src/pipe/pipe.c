#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "configuration.h"
#include "configuration_adv.h"

#include "helper.h"
#include "llnode.h"
#include "array.h"
#include "pipe.h"

void pipe_new(struct pipe **ptr, const char *name)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(*ptr == NULL);

	if (name == NULL)
		abort();

	struct pipe *new = malloc(sizeof(struct pipe));
	if (new == NULL)
		abort();

	// Copy pipe name

	size_t len = strlen(name) + 1;
	new->name = malloc(sizeof(char) * len);

	void *rp = strcpy(new->name, name);

	if(rp != new->name)
		abort();

	// Create the pipe

	int rc = mkfifo(new->name, 0600);
	if (rc == -1){
		perror("ERROR");
		exit(1);
	}

	new->fd = -1;

	*ptr = new;
}

void pipe_free(struct pipe *ptr)
{
	if (ptr == NULL)
		return;

	if (ptr->fd != -1)
		close(ptr->fd);

	remove(ptr->name);
	free(ptr->name);
	free(ptr);
}

void pipe_read()
{
}