#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "configuration.h"
#include "configuration_adv.h"

#include "helper.h"
#include "llnode.h"
#include "array.h"
#include "command.h"

struct cmd
{
	char name[16];
	bool exact_length;
};

struct cmd commands[] = {
	{"invalid ", false},
	{"issueJob ", false},
	{"setConcurrency ", false},
	{"stop ", false},
	{"poll running ", true},
	{"poll queued ", true},
	{"exit", true}
};

int command_recognize(struct array *arr)
{
	char *input = array_get(arr, 0);

	for (int i = cmd_issueJob; i <= cmd_exit; i++) {
		size_t len = strlen(commands[i].name);
		if (commands[i].exact_length)
			len++;

		bool length_correct = (len == array_get_elementsize(arr) * array_get_size(arr));
		if (!commands[i].exact_length)
			length_correct = (len <= array_get_elementsize(arr) * array_get_size(arr));

		if (!length_correct)
			continue;

		int rc = memcmp(commands[i].name, input, len);
		if (rc == 0)
			return i;
	}

	return cmd_invalid;
}

void command_strip(struct array *arr, struct array **ret)
{
	OPTIONAL_ASSERT(arr != NULL);
	OPTIONAL_ASSERT(ret != NULL);
	OPTIONAL_ASSERT(*ret == NULL);

	size_t command_length = strlen(commands[command_recognize(arr)].name);
	if (command_recognize(arr) == cmd_invalid)
		command_length = 0;

	size_t skip = command_length;
	while (1) {
		char *tmp = array_get(arr, skip);
		if (tmp == NULL)
			break;

		if (!isspace(*tmp))
			break;

		skip++;
	}

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);


	for (size_t i = skip; 1; i++) {
		char *tmp = array_get(arr, i);
		if (tmp == NULL)
			break;

		llnode_add(&ll, tmp);
	}

	array_new(ret, ll);
	llnode_free(ll);
}