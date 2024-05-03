#ifndef SYSPROG24_1_ARRAY_H
#define SYSPROG24_1_ARRAY_H

#include "configuration_adv.h"
#include "llnode.h"

struct array
{
	void *data;
	size_t element_size;
	size_t size;
};
void array_free(struct array *ptr);
void array_new(struct array **ptr, struct llnode *ll);
void *array_get(struct array *ptr, size_t pos);
#endif /*SYSPROG24_1_ARRAY_H*/