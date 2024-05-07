#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "configuration.h"

#include "llnode.h"
#include "array.h"
#include "fifopipe.h"
#include "packet.h"
#include "command.h"

void print_txtreadretry(int retries, int retries_max)
{
	switch (retries) {
		case 0:
			break;
		case 1:
			printf("Waiting for %s\n", TXT_NAME);
			break;
		default:
			printf("%d / %d\n", retries, retries_max);
	}
}

void wait_for_txt(int n)
{
	for (int i = 0; i < n; i++) {
		if (access(TXT_NAME, F_OK) == 0)
			return;

		print_txtreadretry(i, n);
		sleep(1);
	}
}

int main(int argc, char *argv[])
{
	// Create Server Process if missing
	if (access(TXT_NAME, F_OK) != 0) {
		pid_t pid = fork();
		if (pid < 0)
			abort();

		int rc = 0;
		if (pid == 0)
			rc = execl(SERVER_NAME, SERVER_NAME, NULL);
		if (rc == -1) {
			perror("ERROR");
			exit(1);
		}
	}

	wait_for_txt(10);

	// Initialize pipes
	struct wopipe *to_exec = NULL;
	wopipe_new(&to_exec, CMD_TO_EXEC);
	struct ropipe *from_exec = NULL;
	ropipe_new(&from_exec, EXEC_TO_CMD);

	// Parse args
	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);

	for (int i = 1; i < argc; i++) {
		for(size_t j = 0; j < strlen(argv[i]); j++) {
			llnode_add(&ll, &(argv[i][j]));
		}
		// Add back space character between arguments
		if (i + 1 < argc) {
			char tmp = ' ';
			llnode_add(&ll, &tmp);
		}
	}
	// Add back null character to denote end of string
	{
		char tmp = '\0';
		llnode_add(&ll, &tmp);
	}

	struct array *arr = NULL;
	array_new(&arr, ll);
	llnode_free(ll);

	if (command_recognize(arr) == cmd_invalid)
		fprintf(stderr, "Invalid Command\n");

	struct packets *p = NULL;
	packets_new(&p);
	packets_pack(p, arr);

	// Send
	packets_send(p, to_exec);
	packets_free(p);

	array_free(arr);

	// Receive reply

	struct array *reply = NULL;
	int retry = 0;
	do {
		array_free(reply);
		reply = NULL;

		p = NULL;
		packets_new(&p);
		packets_receive(p, from_exec);
		packets_unpack(p, &reply);
		packets_free(p);
		retry++;
	} while (array_get(reply, 0) == NULL && retry < 10000);

	char *str = (char *) array_get(reply, 0);
	if (str != NULL)
		printf("%s\n", str);

	array_free(reply);

	wopipe_free(to_exec);
	ropipe_free(from_exec);
	return 0;
}