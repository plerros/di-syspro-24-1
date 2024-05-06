#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"
#include "command.h"
#include "taskboard.h"

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
	struct taskboard *tboard;
	unsigned int concurrency;
};

unsigned int array_to_u(struct array *stripped)
{
	long long tmp = strtol((char *)array_get(stripped, 0), NULL, 10);
	if (tmp < 0)
		return 0;

	if (tmp > UINT_MAX)
		tmp = UINT_MAX;

	return ((unsigned int)tmp);
}

void update_concurrency(unsigned int *concurrency, struct array *stripped)
{
	long long tmp = strtol((char *)array_get(stripped, 0), NULL, 10);
	if (tmp < 0)
		return;

	if (tmp > UINT_MAX)
		tmp = UINT_MAX;

	*concurrency = (unsigned int)tmp;
}

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
			taskboard_add(exd->tboard, stripped);
			break;

		case cmd_setConcurrency:
			update_concurrency(&(exd->concurrency), stripped);
			printf("New concurrency: %u\n", exd->concurrency);
			break;

		case cmd_stop:
			taskboard_remove_tid(exd->tboard, array_to_u(stripped));
			break;

		case cmd_pollrunning:
			taskboard_get_running(exd->tboard, NULL);
			break;

		case cmd_pollqueued:
			taskboard_get_waiting(exd->tboard, NULL);
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
	exd.from_cmd = NULL;
	exd.to_cmd   = NULL;

	// Initialize named pipes
	ropipe_new(&(exd.from_cmd), CMD_TO_EXEC);
	wopipe_new(&(exd.to_cmd), EXEC_TO_CMD);

	exd.tboard = NULL;
	taskboard_new(&(exd.tboard));

	exd.concurrency = 1;

	exd.exit_flag = false;
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

	taskboard_free(exd.tboard);
	ropipe_free(exd.from_cmd);
	wopipe_free(exd.to_cmd);
	remove(TXT_NAME);
	return 0;
}