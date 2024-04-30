#include <stdio.h>
#include <stddef.h>
#include "pipe.h"

#define PIPE_NAME "mypipe"

int main()
{
	struct pipe *pipe_read = NULL;
	pipe_new(&pipe_read, PIPE_NAME);

	printf("%s\n", pipe_read->name);
	pipe_free(pipe_read);

	return 0;
}