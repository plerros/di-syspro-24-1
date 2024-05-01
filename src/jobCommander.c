#include <stdio.h>
#include <stddef.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"

int main()
{
	struct fifopipe *pipe = NULL;
	pipe_new(&pipe, PIPE_NAME);
	printf("%s\n", pipe->name);
	pipe_free(pipe);


	return 0;
}