#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>


#include "configuration.h"

#include "helper.h"
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

struct taskboard *global_tboard = NULL;

void sigchld_handler(__attribute__((unused))int sig)
{
	pid_t pid;
	while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		taskboard_remove_pid(global_tboard, pid);
	}
}

void executor_processcmd(struct executor_data *exd, struct array *command)
{
	if (command == NULL)
		return;

	struct array *stripped = NULL;
	command_strip(command, &stripped);

	array_print_str(command);
	array_print_str(stripped);

	struct array *reply = NULL;

	switch (command_recognize(command)) {
		case cmd_empty:
			goto skip;

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
			taskboard_get_running(exd->tboard, &reply);
			break;

		case cmd_pollqueued:
			taskboard_get_waiting(exd->tboard, &reply);
			break;

		case cmd_exit:
			exd->exit_flag = true;
			break;

		default:
			abort();
	}
	if (array_get(reply, 0) == NULL) {
		array_free(reply);
		reply = NULL;

		struct llnode *ll = NULL;
		char ack[] = "ack";
		llnode_new(&ll, sizeof(char), NULL);

		for (size_t i = 0; i < strlen(ack) + 1; i++)
			llnode_add(&ll, &(ack[i]));

		array_new(&reply, ll);
		llnode_free(ll);
	}

	struct packets *p = NULL;
	packets_new(&p);
	packets_pack(p, reply);
	packets_send(p, exd->to_cmd);
	packets_free(p);
	array_free(reply);

skip:
	array_free(stripped);
}

void mkfifo_werr(char *str)
{
	int rc = mkfifo(str, 0600);
	if (rc == -1){
		perror("ERROR");
		exit(1);
	}
}

void assign_work(struct executor_data *exd)
{
	sigset_t oldmask;
	block_sigchild(&oldmask);

	if (taskboard_get_running(exd->tboard, NULL) < exd->concurrency)
		taskboard_run_next(exd->tboard);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

int main()
{
	// Set up signal handling
	struct sigaction sa;
	{ // SIGCHLD
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = sigchld_handler;
		sigaction(SIGCHLD, &sa, NULL);
	}
	signal(SIGPIPE, SIG_IGN);

	create_txt();

	struct executor_data exd;
	exd.from_cmd = NULL;
	exd.to_cmd   = NULL;

	// Initialize named pipes
	mkfifo_werr(CMD_TO_EXEC);
	mkfifo_werr(EXEC_TO_CMD);
	ropipe_new(&(exd.from_cmd), CMD_TO_EXEC);
	wopipe_new(&(exd.to_cmd), EXEC_TO_CMD);

	exd.tboard = NULL;
	taskboard_new(&(exd.tboard));
	global_tboard = exd.tboard;

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

		assign_work(&exd);

		array_free(command);
	}

	global_tboard = NULL;
	taskboard_free(exd.tboard);
	ropipe_free(exd.from_cmd);
	wopipe_free(exd.to_cmd);
	remove(CMD_TO_EXEC);
	remove(EXEC_TO_CMD);
	remove(TXT_NAME);
	return 0;
}