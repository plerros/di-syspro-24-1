#include <stdio.h>
#include <stddef.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"
#include "command.h"
#include <unistd.h>

int main()
{
	struct ropipe *pipe = NULL;
	ropipe_new(&pipe, PIPE_NAME);

	bool exit_flag = false;

	while (!exit_flag) {

		struct packets *p = NULL;
		packets_new(&p);
		packets_receive(p, pipe);

		struct array *arr = NULL;
		packets_unpack(p, &arr);
		packets_free(p);

		if (arr != NULL) {
			char *str = (char*) array_get(arr, 0);
			if (str != NULL)
				printf("%s\n", str);

			if (cmd_exit == command_recognize(arr)) {
				printf("RECEIVED EXIT\n");
				exit_flag = true;
			}
		}

		array_free(arr);
	}

	ropipe_free(pipe);
	return 0;
}