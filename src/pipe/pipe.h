#ifndef SYSPROG24_1_PIPE_H
#define SYSPROG24_1_PIPE_H

struct pipe
{
	char *name;
	int fd;
};

void pipe_new(struct pipe **ptr, const char *name);
void pipe_free(struct pipe *ptr);
#endif /*SYSPROG24_1_PIPE_H*/