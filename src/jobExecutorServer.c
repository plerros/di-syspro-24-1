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

		struct array *command = NULL;
		struct array *stripped = NULL;
		packets_unpack(p, &command);
		packets_free(p);

		if (command == NULL)
			continue;

		command_strip(command, &stripped);

		array_print_str(command);
		array_print_str(stripped);

		switch (command_recognize(command)) {
			case cmd_empty:
				break;
			case cmd_invalid:
				fprintf(stderr, "Invalid Command\n");
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
				printf("unaccounted %d\n", command_recognize(command));
				abort();
		}

		array_free(stripped);
		array_free(command);
	}

	ropipe_free(from_cmd);
	wopipe_free(to_cmd);
	remove(TXT_NAME);
	return 0;
}