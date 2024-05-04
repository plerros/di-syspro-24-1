#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "array.h"
#include "command.h"

int command_recognize(struct array *arr)
{
	char *commands[] = {"invalid", "issueJob", "setConcurrency", "stop", "poll", "exit"};
	char *input = array_get(arr, 0);

	for (int i = cmd_issueJob; i <= cmd_exit; i++) {
		size_t len = strlen(commands[i]) + 1;
		if (len != array_get_elementsize(arr) * array_get_size(arr))
			continue;

		int rc = memcmp(commands[i], input, len);
		if (rc == 0)
			return i;
	}

	return cmd_invalid;
}