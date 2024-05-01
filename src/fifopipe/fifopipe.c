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

	struct array *packets = NULL;
	pack(src, &packets);

	char *data = (char*)packets->data;

	for (size_t i = 0; i < packets->size; i += 1)
		write(ptr->fd, &(data[i * packets->element_size]), PACKET_SIZE);

	close(ptr->fd);
	ptr->fd = -1;

	array_free(packets);
}

static void pipe_read(struct fifopipe *ptr, struct array **dst)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(dst != NULL);
	OPTIONAL_ASSERT(*dst == NULL);

	if(ptr->fd == -1)
		ptr->fd = open(ptr->name, O_RDONLY);

	if(ptr->fd == -1)
		abort();

	struct packet pack;
	read(ptr->fd, &pack, PACKET_SIZE);

	struct llnode *ll = NULL;
	llnode_new(&ll, PACKET_SIZE, NULL);
	llnode_add(&ll, &pack);

	size_t elements = pack.data_sum / pack.element_size;
	elements += pack.data_sum % pack.element_size;

	for (size_t i = 1; i < elements; i++) {
		read(ptr->fd, &pack, PACKET_SIZE);
		llnode_add(&ll, &pack);
	}

	close(ptr->fd);
	ptr->fd = -1;

	struct array *packets = NULL;
	array_new(&packets, ll);
	llnode_free(ll);

	unpack(packets, dst);
	array_free(packets);
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

void ropipe_read(struct ropipe *ptr, struct array **dst)
{
	OPTIONAL_ASSERT(ptr != NULL);
	pipe_read(ptr->pipe, dst);
}