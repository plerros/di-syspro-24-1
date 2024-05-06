#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

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

	for (size_t i = 0; i < size; i++)
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

	for (size_t i = 0; i < size; i++) {
		tmp = (struct task *)llnode_get(ptr->tasks, i);
		if (tmp == NULL)
			continue;

		if (task_iswaiting(tmp)) {
			printf("job_%d\n", tmp->taskid);
		}
	}

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
	for (size_t i = 0; i < size; i++) {
		tmp = (struct task *)llnode_get(ptr->tasks, i);
		if (task_isrunning(tmp))
			printf("job_%d\n", tmp->taskid);
	}

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}
