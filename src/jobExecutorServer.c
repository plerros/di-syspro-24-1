#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"
#include "command.h"
#include <unistd.h>

int main()
{
	// Create jobExecutorServer.txt
	FILE *txt;
	txt = fopen(TXT_NAME, "w");
	fprintf(txt, "%d", getpid());
	fclose(txt);

	// Initialize named pipes
	struct ropipe *from_cmd = NULL;
	ropipe_new(&from_cmd, CMD_TO_EXEC);
	struct wopipe *to_cmd = NULL;
	wopipe_new(&to_cmd, EXEC_TO_CMD);

	bool exit_flag = false;

	while (!exit_flag) {

		struct packets *p = NULL;
		packets_new(&p);
		packets_receive(p, from_cmd);

		struct array *arr = NULL;
		packets_unpack(p, &arr);
		packets_free(p);

		if (arr == NULL)
			continue;

		char *str = (char*) array_get(arr, 0);
		if (str != NULL)
			printf("%s\n", str);

		struct array *arr2 = NULL;

		command_strip(arr, &arr2);

		char *str2 = (char*) array_get(arr2, 0);
		if (str2 != NULL)
			printf("%s\n", str2);

		switch (command_recognize(arr)) {
			case cmd_invalid:
				break;
			case cmd_issueJob:
				break;
			case cmd_setConcurrency:
				break;
			case cmd_stop:
				break;
			case cmd_pollrunning:
				break;
			case cmd_pollqueued:
				break;
			case cmd_exit:
				exit_flag = true;
				break;
			default:
				abort();
		}

		array_free(arr2);
		array_free(arr);
	}

	ropipe_free(from_cmd);
	wopipe_free(to_cmd);
	remove(TXT_NAME);
	return 0;
}