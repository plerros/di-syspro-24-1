#include "configuration.h"
#include "configuration_adv.h"

#include "helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "llnode.h"

void array_free(struct array *ptr)
{
	if (ptr == NULL)
		return;

	if (ptr->data != NULL)
		free(ptr->data);

	free(ptr);
}

void array_new(struct array **ptr, struct llnode *ll)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(*ptr == NULL);

	if (ptr == NULL)
		return;

	struct array *new = malloc(sizeof(struct array));
	if (new == NULL)
		abort();

	new->data = NULL;
	new->element_size = 0;
	new->size = 0;

	if (ll != NULL) {
		new->element_size = ll->element_size;
		new->size = llnode_getsize(ll);
	}

	void *arr = malloc(new->element_size * new->size);
	if (arr == NULL && new->element_size * new->size > 0)
		abort();

	// copy
	for (size_t source_offset = new->element_size * new->size; ll != NULL; ll = ll->next) {
		size_t llnode_size = ll->logical_size * ll->element_size;
		
		source_offset -= llnode_size;
		memcpy(arr + source_offset, ll->data, llnode_size);
	}
	new->data = arr;

	*ptr = new;
	return;
}
