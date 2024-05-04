#ifndef SYSPROG24_1_TASKBOARD_H
#define SYSPROG24_1_TASKBOARD_H

struct taskboard
{
	struct array *queued;
	struct array *running;
}

#endif /*SYSPROG24_1_TASKBOARD_H*/