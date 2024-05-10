#include <stdbool.h>
#include <stdlib.h>

#include "queue.h"

static struct queue *gnext(struct queue *ptr)
{
	if (ptr == NULL)
		return NULL;

	return ptr->next;
}

static struct queue *gprev(struct queue *ptr)
{
	if (ptr == NULL)
		return NULL;

	return ptr->prev;
}

static void snext(struct queue *ptr, struct queue *next)
{
	if (ptr == NULL)
		return;

	ptr->next = next;
}

static void sprev(struct queue *ptr, struct queue *prev)
{
	if (ptr == NULL)
		return;

	ptr->prev = prev;
}

static size_t queue_gettaskid(struct queue *ptr)
{
	if (ptr == NULL)
		return 0;
	return ptr->task_id;
}

static pid_t queue_getpid(struct queue *ptr)
{
	if (ptr == NULL)
		return -1;
	return ptr->pid;
}

void queue_push(struct queue **ptr, size_t task_id, pid_t pid)
{
	if (ptr == NULL)
		return;

	struct queue *new = malloc(sizeof(struct queue));
	if (new == NULL)
		abort();

	new->task_id = task_id;
	new->pid = pid;

	snext(new, new);
	sprev(new, new);

	if (*ptr != NULL) {
		snext(new, *ptr);
		sprev(new, gprev(*ptr));

		snext(gprev(new), new);
		sprev(gnext(new), new);
	}

	*ptr = new;
}


size_t queue_pop(struct queue **ptr)
{
	if (ptr == NULL)
		return 0;

	struct queue *rm = gprev(*ptr);

	sprev(gnext(rm), gprev(rm));
	snext(gprev(rm), gnext(rm));

	*ptr = gnext(rm);

	if (gnext(rm) == rm && gprev(rm) == rm)
		*ptr = NULL;

	size_t ret = 0;
	if (rm != NULL)
		ret = rm->task_id;

	free(rm);
	return ret;
}

size_t queue_find_pop(struct queue **ptr, size_t task_id, pid_t pid)
{
	if (ptr == NULL)
		return 0;

	struct queue *tmp = *ptr;
	bool update_ptr = false;

	if (task_id == 0 && pid == -1)
		goto out;

	while (1) {
		if (task_id != 0 && queue_gettaskid(tmp) == task_id)
			break;
		if (pid != -1 && queue_getpid(tmp) == pid)
			break;

		tmp = gnext(tmp);
		if (tmp == *ptr) // not found
			return 0;
	}
	tmp = gnext(tmp); // pop removes (tmp->next)->prev which is tmp

out:
	if (*ptr == tmp)
		update_ptr = true;

	size_t ret = queue_pop(&tmp);

	if (tmp == NULL)
		update_ptr = true;

	if (update_ptr)
		*ptr = tmp;

	return ret;
}