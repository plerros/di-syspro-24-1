#include <stdio.h>
#include <stddef.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"

int main()
{
	struct ropipe *pipe = NULL;
	ropipe_new(&pipe, PIPE_NAME);

	struct packets *p = NULL;
	packets_new(&p);
	packets_receive(p, pipe);


	struct array *arr = NULL;
	packets_unpack(p, &arr);
	packets_free(p);

	char *str = (char*)arr->data;
	printf("%s\n", str);
	array_free(arr);

	ropipe_free(pipe);
	return 0;
}