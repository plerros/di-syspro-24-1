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

void create_txt()
{
	FILE *txt;
	txt = fopen(TXT_NAME, "w");
	fprintf(txt, "%d", getpid());
	fclose(txt);
}

struct executor_data
{
	struct ropipe *from_cmd;
	struct wopipe *to_cmd;
	bool exit_flag;
};

void executor_processcmd(struct executor_data *exd, struct array *command)
{
	if (command == NULL)
		return;

	struct array *stripped = NULL;
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
			exd->exit_flag = true;
			break;
		default:
			abort();
	}

	array_free(stripped);
}

int main()
{
	create_txt();

	struct executor_data exd;
	exd.from_cmd  = NULL;
	exd.to_cmd    = NULL;
	exd.exit_flag = false;

	// Initialize named pipes
	ropipe_new(&(exd.from_cmd), CMD_TO_EXEC);
	wopipe_new(&(exd.to_cmd), EXEC_TO_CMD);

	while (!exd.exit_flag) {
		struct packets *p = NULL;
		packets_new(&p);
		packets_receive(p, exd.from_cmd);

		struct array *command = NULL;
		packets_unpack(p, &command);
		packets_free(p);

		executor_processcmd(&exd, command);

		array_free(command);
	}

	ropipe_free(exd.from_cmd);
	wopipe_free(exd.to_cmd);
	remove(TXT_NAME);
	return 0;
}