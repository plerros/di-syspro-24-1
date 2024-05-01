#include <stdio.h>
#include <stddef.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"


int main()
{
	struct fifopipe *pipe_read = NULL;
	pipe_new(&pipe_read, PIPE_NAME);
	printf("%s\n", pipe_read->name);
	pipe_free(pipe_read);

/*
	char string[] = PIPE_NAME;
	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);
	for (int i = 0; i < 16; i++)
		llnode_add(&ll, &(string[i]));

	char ch = '\0';
	llnode_add(&ll, &(ch));

	struct array *arr = NULL;
	array_new(&arr, ll);
	llnode_free(ll);

	char *out = arr->data;
	printf("1: %s\n", out);

	struct array *packed_arr = NULL;
	struct array *unpacked_arr = NULL;
	pack(arr, &packed_arr);
	array_free(arr);

	unpack(packed_arr, &unpacked_arr);
	array_free(packed_arr);

	out = unpacked_arr->data;
	printf("%s\n", out);
	array_free(unpacked_arr);

*/
	return 0;
}