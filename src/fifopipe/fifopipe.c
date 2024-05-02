#include <errno.h>
#include <fcntl.h>
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
#include "fifopipe.h"
#include "packet.h"

// INTERNAL PIPE FUNCTIONS

static void pipe_new(struct fifopipe **ptr, const char *name)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(*ptr == NULL);

	if (name == NULL)
		abort();

	struct fifopipe *new = malloc(sizeof(struct fifopipe));
	if (new == NULL)
		abort();

	// Copy pipe name

	size_t len = strlen(name) + 1;
	new->name = malloc(sizeof(char) * len);

	void *rp = strcpy(new->name, name);

	if(rp != new->name)
		abort();

	new->fd = -1;

	*ptr = new;
}

static void pipe_free(struct fifopipe *ptr)
{
	if (ptr == NULL)
		return;

	if (ptr->fd != -1)
		close(ptr->fd);

	if (ptr->name != NULL) {
		remove(ptr->name);
		free(ptr->name);
	}

	free(ptr);
}

static void pipe_write(struct fifopipe *ptr, struct array *src)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(src != NULL);

	if(ptr->fd == -1)
		ptr->fd = open(ptr->name, O_WRONLY);

	if(ptr->fd == -1)
		abort();

	char *data = (char*)src->data;

	for (size_t i = 0; i < src->size; i += 1)
		write(ptr->fd, &(data[i * src->element_size]), src->element_size);
}

static void pipe_read(struct fifopipe *ptr, struct array **dst, size_t msg_size, size_t msg_count)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(dst != NULL);
	OPTIONAL_ASSERT(*dst == NULL);

	if (ptr->fd == -1)
		ptr->fd = open(ptr->name, O_RDONLY);

	if (ptr->fd == -1)
		abort();

	struct llnode *ll = NULL;
	llnode_new(&ll, msg_size, NULL);

	char *tmp = malloc(sizeof(char) * msg_size);
	if (tmp == NULL)
		abort();

	for (size_t i = 0; i < msg_count; i++) {
		read(ptr->fd, tmp, msg_size);
		llnode_add(&ll, tmp);
	}

	free(tmp);

	array_new(dst, ll);
	llnode_free(ll);
}

// WRITE ONLY PIPE

void wopipe_new(struct wopipe **ptr, const char *name)
{
	struct wopipe *new = malloc(sizeof(struct wopipe));
	if (new == NULL)
		abort();

	new->pipe = NULL;

	pipe_new(&(new->pipe), name);
	*ptr = new;
}

void wopipe_free(struct wopipe *ptr)
{
	pipe_free(ptr->pipe);
	free(ptr);
}

void wopipe_write(struct wopipe *ptr, struct array *src)
{
	OPTIONAL_ASSERT(ptr != NULL);
	pipe_write(ptr->pipe, src);
}

// READ ONLY PIPE

void ropipe_new(struct ropipe **ptr, const char *name)
{
	struct ropipe *new = malloc(sizeof(struct wopipe));
	if (new == NULL)
		abort();

	new->pipe = NULL;

	pipe_new(&(new->pipe), name);

	// Create the pipe

	int rc = mkfifo(new->pipe->name, 0600);
	if (rc == -1){
		perror("ERROR");
		exit(1);
	}
	*ptr = new;
}

void ropipe_free(struct ropipe *ptr)
{
	char *name = ptr->pipe->name;
	ptr->pipe->name = NULL;

	pipe_free(ptr->pipe);
	remove(name);
	free(name);
	free(ptr);
}

void ropipe_read(struct ropipe *ptr, struct array **dst, size_t msg_size, size_t msg_count)
{
	OPTIONAL_ASSERT(ptr != NULL);
	pipe_read(ptr->pipe, dst, msg_size, msg_count);
}