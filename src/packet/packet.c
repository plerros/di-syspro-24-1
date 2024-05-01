#include <string.h>
#include "helper.h"

#include "configuration.h"

#include "packet.h"
#include "assert.h"

void pack(struct array *ptr, struct array **packets)
{
	static_assert(PACKET_SIZE > 3 * sizeof(size_t), "PACKET_SIZE too small.");
	OPTIONAL_ASSERT(ptr != NULL);

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(struct packet), NULL);

	struct packet tmp;
	tmp.data_sum = ptr->size;
	tmp.index = 0;
	tmp.element_size = ptr->element_size;
	char *src = (char*)ptr->data;

	for (size_t i = 0; i < ptr->size;) {
		size_t size = sizeof(tmp.data);
		if (size >  ptr->size - i)
			size = ptr->size - i;

		memcpy(tmp.data, &(src[i]), size);
		llnode_add(&ll, &tmp);
		tmp.index++;
		i += size;
	}

	array_new(packets, ll);
	llnode_free(ll);
}

void unpack(struct array *packets, struct array **ptr)
{
	OPTIONAL_ASSERT(packets != NULL);

	struct packet *arr = (struct packet*)(packets->data);

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);

	size_t datapack = sizeof(arr[0].data);
	size_t sum = arr[0].data_sum;

	for(size_t i=0, j=0; i * datapack + j < sum;){
		llnode_add(&ll, &(arr[i].data[j]));

		j++;
		if(j >= datapack) {
			i++;
			j = 0;
		}
	}


	array_new(ptr, ll);
	llnode_free(ll);

	(*ptr)->element_size = arr[0].element_size;
}