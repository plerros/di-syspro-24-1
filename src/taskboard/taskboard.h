#ifndef SYSPROG24_1_TASKBOARD_H
#define SYSPROG24_1_TASKBOARD_H

#include "configuration.h"

#include "array.h"

struct taskboard
{
	struct array *tasks;
};

void taskboard_new(struct taskboard **ptr);
void taskboard_free(struct taskboard *ptr);

void taskboard_push(struct taskboard *ptr, struct array *command);
void taskboard_pop_to_run(struct taskboard *ptr);

void taskboard_remove_tid(struct taskboard *ptr, size_t tid, struct array **reply);
void taskboard_remove_pid(struct taskboard *ptr, pid_t pid);
size_t taskboard_get_waiting(struct taskboard *ptr, struct array **waiting);
size_t taskboard_get_running(struct taskboard *ptr, struct array **running);
#endif /*SYSPROG24_1_TASKBOARD_H*/