#ifndef SYSPROG24_1_FIFOPIPE_H
#define SYSPROG24_1_FIFOPIPE_H

struct fifopipe
{
	char *name;
	int fd;
};

void pipe_new(struct fifopipe **ptr, const char *name);
void pipe_free(struct fifopipe *ptr);
#endif /*SYSPROG24_1_FIFOPIPE_H*/