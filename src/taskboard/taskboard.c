#include <linux/limits.h>
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

	*ptr = new;
}

void taskboard_free(struct taskboard *ptr)
{
	if (ptr == NULL)
		return;

	size_t size = array_get_size(ptr->tasks);

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

	array_free(ptr->tasks);
	free(ptr);
}

void taskboard_add(struct taskboard *ptr, struct array *command)
{
	OPTIONAL_ASSERT(ptr != NULL);
	if (ptr == NULL)
		return;

	if (command == NULL)
		return;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	struct task *tmp = NULL;
	task_new(&tmp, command, array_get_size(ptr->tasks));

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(struct task), NULL);

	size_t size = array_get_size(ptr->tasks);
	for (size_t i = 0; i < size; i++)
		llnode_add(&ll, array_get(ptr->tasks, i));

	llnode_add(&ll, tmp);

	array_free(ptr->tasks);
	ptr->tasks = NULL;
	array_new(&(ptr->tasks), ll);
	llnode_free(ll);

	tmp->command = NULL; // It's janky, I'm sorry
	task_free(tmp);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void taskboard_remove_tid(struct taskboard *ptr, size_t tid)
{
	if (ptr == NULL)
		return;

	if (tid >= array_get_size(ptr->tasks))
		return;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	struct task *tmp = (struct task *)array_get(ptr->tasks, tid);
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

	size_t size = array_get_size(ptr->tasks);
	struct task *tmp = NULL;
	for (size_t i = 0; i < size; i++) {
		tmp = (struct task *)array_get(ptr->tasks, i);
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
	sprintf(jobstr, "job_%lu, ", t->taskid);
	concat(str, jobstr);
	concat(str, array_get(t->command, 0));
	sprintf(jobstr, ", %lu\n", queuePosition);
	concat(str, jobstr);
}

size_t taskboard_get_waiting(struct taskboard *ptr, struct array **waiting)
{
	if (ptr == NULL)
		return 0;

	if (ptr->tasks == NULL)
		return 0;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	size_t size = array_get_size(ptr->tasks);
	struct task *tmp = NULL;

	char *str = NULL;

	size_t waiting_position = 0;

	for (size_t i = 0; i < size; i++) {

		tmp = (struct task *)array_get(ptr->tasks, i);
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
		char end = '\0';
		llnode_add(&ll, &(end));
	}

	array_new(waiting, ll);
	llnode_free(ll);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
	return waiting_position;
}

size_t taskboard_get_running(struct taskboard *ptr, struct array **running)
{
	if (ptr == NULL)
		return 0;

	if (ptr->tasks == NULL)
		return 0;

	sigset_t oldmask;
	block_sigchild(&oldmask);

	size_t size = array_get_size(ptr->tasks);
	struct task *tmp = NULL;

	char *str = NULL;
	size_t running_position = 0;

	for (size_t i = 0; i < size; i++) {
		tmp = (struct task *)array_get(ptr->tasks, i);
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
		char end = '\0';
		llnode_add(&ll, &(end));
	}

	if (running != NULL)
		array_new(running, ll);

	llnode_free(ll);


	sigprocmask(SIG_SETMASK, &oldmask, NULL);
	return running_position;
}

void taskboard_run_next(struct taskboard *ptr)
{
	if (ptr == NULL)
		return;

	if (ptr->tasks == NULL)
		return;

	sigset_t oldmask;
	block_sigchild(&oldmask);


	size_t size = array_get_size(ptr->tasks);
	struct task *tmp = NULL;

	for (size_t i = 0; i < size; i++) {
		tmp = (struct task *)array_get(ptr->tasks, i);
		if (!task_iswaiting(tmp))
			continue;
	}


	if (task_iswaiting(tmp)) {
		pid_t pid = fork();
		if (pid < 0)
			abort();

		int rc = 0;
		if (pid == 0) {
			fclose(stdout);
			fclose(stderr);

			// Add current working directory to path
			char *path = getenv("PATH");
			char cwd[PATH_MAX];
			getcwd(cwd, sizeof(cwd));
			char separator[] = ":";

			char *newpath = malloc(strlen(path) + strlen(separator) + strlen(cwd) + 1);

			strcpy(newpath, path);
			strcat(newpath, separator);
			strcat(newpath, cwd);

			setenv("PATH", newpath, 1);
			rc = execlp("sh", "sh", "-c", (char *)array_get(tmp->command, 0), NULL);
		}
		else {
			tmp->pid = pid;
		}

		if (rc == -1) {
			printf("COMMAND FAILED: %s\n", (char *)array_get(tmp->command, 0));
			perror("ERROR");
			exit(1);
		}
	}

	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}
