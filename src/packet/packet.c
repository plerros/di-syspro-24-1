#include <string.h>
#include <stdlib.h>

#include "configuration.h"

#include "helper.h"
#include "packet.h"
#include "assert.h"

struct packet
{
	size_t data_sum;
	size_t element_size; // of the source data
	size_t index;
	char data[PACKET_SIZE - (3 * sizeof(size_t))];
};

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

bool packet_isnext(struct packet *a, struct packet *b)
{
	OPTIONAL_ASSERT(a != NULL);
	OPTIONAL_ASSERT(b != NULL);

	if (a->data_sum != b->data_sum)
		return false;

	if (a->element_size != b->element_size)
		return false;

	if (! (a->index + 1 == b->index))
		return false;

	return true;
}

void packets_new(struct packets **ptr)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(*ptr == NULL);

	struct packets *new = malloc(sizeof(struct packets));
	if (new == NULL)
		abort();

	new->packets = NULL;
	*ptr = new;
}

void packets_free(struct packets *ptr)
{
	if (ptr == NULL)
		return;

	if (ptr->packets != NULL)
		array_free(ptr->packets);

	free(ptr);
}

void packets_pack(struct packets *ptr, struct array *src)
{
	OPTIONAL_ASSERT(ptr != NULL);
	pack(src, &(ptr->packets));
}

void packets_unpack(struct packets *ptr, struct array **dst)
{
	OPTIONAL_ASSERT(ptr != NULL);
	unpack(ptr->packets, dst);
}

void packets_send(struct packets *ptr, struct wopipe *pipe)
{
	OPTIONAL_ASSERT(ptr != NULL);
	wopipe_write(pipe, ptr->packets);
}

void packets_receive(struct packets *ptr, struct ropipe *pipe)
{
	OPTIONAL_ASSERT(ptr != NULL);

	struct array *tmp = NULL;

	ropipe_read(pipe, &tmp, PACKET_SIZE, 1);

	struct packet *packet0 = (struct packet *)tmp->data;
	size_t packet_count = DIV_ROOF(packet0->data_sum, sizeof(packet0->data));

	struct packet *packet = malloc(sizeof(struct packet) * packet_count);
	memcpy(&(packet[0]), tmp->data, sizeof(struct packet));
	array_free(tmp);
	tmp = NULL;

	ropipe_read(pipe, &tmp, PACKET_SIZE, packet_count);
	memcpy(&(packet[1]), tmp->data, sizeof(struct packet) * (packet_count - 1));
	array_free(tmp);

	struct llnode *ll = NULL;
	llnode_new(&ll, PACKET_SIZE, NULL);
	llnode_add(&ll, &(packet[0]));

	for (size_t i = 1; i < packet_count; i++) {
		if (!(packet_isnext(&(packet[i-1]), &(packet[i]))))
			abort();
		llnode_add(&ll, &(packet[i]));
	}

	free(packet);

	array_new(&(ptr->packets), ll);
	llnode_free(ll);
}