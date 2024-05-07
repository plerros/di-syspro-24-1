#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configuration.h"

#include "helper.h"
#include "task.h"
#include "taskboard.h"

void taskboard_new(struct taskboard **ptr)
{

	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(*ptr == NULL);

	if (ptr == NULL)
		return;

	struct taskboard *new = malloc(sizeof(struct taskboard));
	if (new == NULL)
		abort();

	new->tasks = NULL;
	llnode_new(&(new->tasks), sizeof(struct task), NULL);

	*ptr = new;
}

void taskboard_free(struct taskboard *ptr)
{
	if (ptr == NULL)
		return;

	size_t size = llnode_getsize(ptr->tasks);

	/*
	 * Note to self:
	 * 
	 * We can't call task_free().
	 * If we did, llnode_free() would free the tasks twice!
	 * 
	 * Thankfully task_end() does exactly what we want.
	 * Free all pointers and set them to 0 without freeing the object.
	 * 
	 * I'm sorry, this is extremely janky.
	 */

	for (unsigned int i = 0; i < size; i++)
		taskboard_remove_tid(ptr, i);

	llnode_free(ptr->tasks);
	free(ptr);
}

void taskboard_add(struct taskboard *ptr, struct array *command)
{
	OPTIONAL_ASSERT(ptr != NULL);
	if (ptr == NULL)
		return;

	if (command == NULL)
		return;

	struct task *tmp = NULL;
	task_new(&tmp, command, (unsigned int)llnode_getsize(ptr->tasks));
	llnode_add(&(ptr->tasks), tmp);

	tmp->command = NULL; // It's janky, I'm sorry
	task_free(tmp);
}

void taskboard_remove_tid(struct taskboard *ptr, unsigned int tid)
{
	if (ptr == NULL)
		return;

	if (tid >= llnode_getsize(ptr->tasks))
		return;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	struct task *tmp = (struct task *)llnode_get(ptr->tasks, tid);
	if (tmp == NULL)
		abort();

	task_end(tmp);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void taskboard_remove_pid(struct taskboard *ptr, pid_t pid)
{
	if (ptr == NULL)
		return;

	if (ptr->tasks == NULL)
		return;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	size_t size = llnode_getsize(ptr->tasks);
	struct task *tmp = NULL;
	for (size_t i = 0; i < size; i++) {
		tmp = (struct task *)llnode_get(ptr->tasks, i);
		if (tmp->pid != pid)
			tmp = NULL;
		else
			break;
	}
	task_end(tmp);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

static void concat(char **str1, char *str2)
{
	if (*str1 == NULL && str2 == NULL) {
		return;
	}

	if (*str1 != NULL && str2 == NULL) {
		return;
	}

	size_t len1 = 0;
	size_t len2 = 0;

	if (*str1 != NULL)
		len1 = strlen(*str1);
	if (str2 != NULL)
		len2 = strlen(str2);
/*
	if (len1 + len2 == 0)
		abort();

	char *new = malloc(len1 + len2 + 1);
	if (new == NULL)
		abort();

	new[0] = '\0';

	if (*str1 != NULL) {
		strcpy(new, *str1);
		free(*str1);
	}

	if (str2 != NULL)
		strcat(new, str2);

	*str1 = new;
*/


	if (*str1 == NULL && str2 != NULL) {
		char *new = malloc(len1 + len2 + 1);
		if (new == NULL)
			abort();
		strcpy(new, str2);
		*str1 = new;
		return;
	}

	if (*str1 != NULL && str2 != NULL) {
		char *new = malloc(len1 + len2 + 1);
		if (new == NULL)
			abort();

		strcpy(new, *str1);
		free(*str1);
		strcat(new, str2);
		*str1 = new;
		return;
	}
}

static void concat_task(struct task *t, char **str, size_t queuePosition)
{
	char jobstr[1024];
	sprintf(jobstr, "job_%d, ", t->taskid);
	concat(str, jobstr);
	concat(str, array_get(t->command, 0));
	sprintf(jobstr, ", %lu\n", queuePosition);
	concat(str, jobstr);
}

void taskboard_get_waiting(struct taskboard *ptr, struct array **waiting)
{
	if (ptr == NULL)
		return;

	if (ptr->tasks == NULL)
		return;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	size_t size = llnode_getsize(ptr->tasks);
	struct task *tmp = NULL;

	char *str = NULL;

	size_t waiting_position = 0;

	for (size_t i = 0; i < size; i++) {

		tmp = (struct task *)llnode_get(ptr->tasks, i);
		if (tmp == NULL)
			continue;

		if (task_iswaiting(tmp)) {
			concat_task(tmp, &str, waiting_position);
			waiting_position++;
		}

	}

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);

	if (str != NULL) {
		for(size_t i = 0; i < strlen(str); i++)
			llnode_add(&ll, &(str[i]));
		free(str);
	}

	array_new(waiting, ll);
	llnode_free(ll);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void taskboard_get_running(struct taskboard *ptr, struct array **running)
{
	if (ptr == NULL)
		return;

	if (ptr->tasks == NULL)
		return;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	size_t size = llnode_getsize(ptr->tasks);
	struct task *tmp = NULL;

	char *str = NULL;
	size_t running_position = 0;

	for (size_t i = 0; i < size; i++) {
		tmp = (struct task *)llnode_get(ptr->tasks, i);
		if (task_isrunning(tmp)) {
			concat_task(tmp, &str, running_position);
			running_position++;
		}
	}

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);

	if (str != NULL) {
		for(size_t i = 0; i < strlen(str); i++)
			llnode_add(&ll, &(str[i]));
		free(str);
	}

	array_new(running, ll);
	llnode_free(ll);


	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}
