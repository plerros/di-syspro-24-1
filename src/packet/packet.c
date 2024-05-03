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

	OPTIONAL_ASSERT(packets != NULL);
	OPTIONAL_ASSERT(*packets == NULL);


	if (packets == NULL)
		return;

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(struct packet), NULL);

	size_t size = 0;
	size_t element_size = 0;
	char *data = NULL;

	if (ptr != NULL) {
		size = ptr->size;
		element_size = ptr->element_size;
		data = (char*)array_get(ptr, 0);
	}

	struct packet tmp;
	tmp.data_sum = size;
	tmp.index = 0;
	tmp.element_size = element_size;

	for (size_t i = 0; i < size;) {
		size_t current_size = sizeof(tmp.data);
		if (current_size >  size - i)
			current_size = size - i;

		if (data != NULL) {
			memset(tmp.data, 0, sizeof(tmp.data));
			memcpy(tmp.data, &(data[i]), current_size);
			llnode_add(&ll, &tmp);
		}
		tmp.index++;
		i += current_size;
	}

	array_new(packets, ll);
	llnode_free(ll);
}

void unpack(struct array *packets, struct array **ptr)
{
	OPTIONAL_ASSERT(ptr != NULL);
	OPTIONAL_ASSERT(*ptr == NULL);

	if (ptr == NULL)
		return;

	struct packet *arr = NULL;

	if (packets != NULL)
		arr = (struct packet*)(array_get(packets, 0));

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);

	size_t datapack = 0;
	size_t sum = 0;
	size_t element_size = 0;

	if (arr != NULL) {
		datapack = sizeof(arr[0].data);
		sum = arr[0].data_sum;
		element_size = arr[0].element_size;
	}

	for (size_t i=0, j=0; i * datapack + j < sum;){
		llnode_add(&ll, &(arr[i].data[j]));

		j++;
		if (j >= datapack) {
			i++;
			j = 0;
		}
	}

	array_new(ptr, ll);
	llnode_free(ll);

	(*ptr)->element_size = element_size;
}

bool packet_isnext(struct packet *a, struct packet *b)
{
	OPTIONAL_ASSERT(a != NULL);
	OPTIONAL_ASSERT(b != NULL);

// TODO

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

	if (ptr == NULL)
		return;

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

	struct array **packets = NULL;

	if (ptr != NULL)
		packets = &(ptr->packets);

	pack(src, packets);
}

void packets_unpack(struct packets *ptr, struct array **dst)
{
	OPTIONAL_ASSERT(ptr != NULL);
	struct array *packets = NULL;

	if (ptr != NULL)
		packets = ptr->packets;

	unpack(packets, dst);
}

void packets_send(struct packets *ptr, struct wopipe *pipe)
{
	OPTIONAL_ASSERT(ptr != NULL);
	struct array *packets = NULL;

	if (ptr != NULL)
		packets = ptr->packets;

	wopipe_write(pipe, packets);
}

void packets_receive(struct packets *ptr, struct ropipe *pipe)
{
	OPTIONAL_ASSERT(ptr != NULL);

	struct array **dst = NULL;
	if (ptr != NULL)
		dst = &(ptr->packets);

	struct array *tmp = NULL;

	ropipe_read(pipe, &tmp, PACKET_SIZE, 1);

	struct packet *packet0 = NULL;
	size_t packet_count = 0;
	struct packet *packet = NULL;

	if (tmp != NULL)
		packet0 = (struct packet *)array_get(tmp, 0);

	if (packet0 == NULL)
		goto skip;

	packet_count = DIV_ROOF(packet0->data_sum, sizeof(packet0->data));
	if (packet_count == 0)
		goto skip;

	packet = malloc(sizeof(struct packet) * packet_count);
	if (packet == NULL)
		abort();

	memcpy(&(packet[0]), packet0, sizeof(struct packet));

	array_free(tmp);
	tmp = NULL;

	ropipe_read(pipe, &tmp, PACKET_SIZE, packet_count);

	struct pakcet *packet1 = NULL;


	if (
		tmp != NULL
		&& array_get(tmp, 0) != NULL
		&& packet_count > 1
	) {
		memcpy(&(packet[1]), array_get(tmp, 0), sizeof(struct packet) * (packet_count - 1));
	}

skip:
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

	array_new(dst, ll);
	llnode_free(ll);
}