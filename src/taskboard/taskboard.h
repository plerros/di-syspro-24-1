#ifndef SYSPROG24_1_TASKBOARD_H
#define SYSPROG24_1_TASKBOARD_H

#include "configuration.h"

#include "llnode.h"

struct taskboard
{
	struct llnode *tasks;
};

void taskboard_new(struct taskboard **ptr);
void taskboard_free(struct taskboard *ptr);
void taskboard_add(struct taskboard *ptr, struct array *command);
void taskboard_remove_tid(struct taskboard *ptr, unsigned int tid);
void taskboard_remove_pid(struct taskboard *ptr, pid_t pid);
void taskboard_get_waiting(struct taskboard *ptr, struct array **waiting);
void taskboard_get_running(struct taskboard *ptr, struct array **running);
#endif /*SYSPROG24_1_TASKBOARD_H*/