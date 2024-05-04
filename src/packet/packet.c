#include <string.h>
#include <stdlib.h>

#include "configuration.h"

#include "helper.h"
#include "packet.h"
#include "assert.h"

#define PAKET_DATA_SIZE (PACKET_SIZE - (3 * sizeof(size_t)))

struct packet
{
	size_t data_sum;
	size_t element_size; // of the source data
	size_t index;
	char data[PAKET_DATA_SIZE];
};


size_t packet_get_datasum(struct packet *ptr)
{
	if (ptr == NULL)
		return 0;

	return ptr->data_sum;
}

size_t packet_get_elementsize(struct packet *ptr)
{
	if (ptr == NULL)
		return 0;

	return ptr->element_size;
}

size_t packet_get_index(struct packet *ptr)
{
	if (ptr == NULL)
		return 0;

	return ptr->index;
}

char *packet_get_data(struct packet *ptr)
{
	if (ptr == NULL)
		return 0;

	return &(ptr->data[0]);
}

size_t packet_get_packetcount(struct packet *ptr)
{
	if (ptr == NULL)
		return 0;

	return (DIV_ROOF(ptr->data_sum, PAKET_DATA_SIZE));
}

void pack(struct array *ptr, struct array **packets)
{
	static_assert(PACKET_SIZE > 3 * sizeof(size_t), "PACKET_SIZE too small.");

	OPTIONAL_ASSERT(packets != NULL);
	OPTIONAL_ASSERT(*packets == NULL);

	if (packets == NULL)
		return;

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(struct packet), NULL);

	char *data = (char*)array_get(ptr, 0);

	struct packet tmp;
	tmp.data_sum = array_get_size(ptr);
	tmp.index = 0;
	tmp.element_size = array_get_elementsize(ptr);

	for (size_t i = 0; i < tmp.data_sum;) {
		size_t current_size = PAKET_DATA_SIZE;
		if (current_size >  tmp.data_sum - i)
			current_size = tmp.data_sum - i;

		if (data != NULL) {
			memset(tmp.data, 0, PAKET_DATA_SIZE);
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

	struct packet *arr = (struct packet*)(array_get(packets, 0));

	struct llnode *ll = NULL;
	llnode_new(&ll, sizeof(char), NULL);

	size_t sum = packet_get_datasum(&arr[0]);

	for (size_t i=0, j=0; i * PAKET_DATA_SIZE + j < sum;){
		llnode_add(&ll, &(arr[i].data[j]));

		j++;
		if (j >= PAKET_DATA_SIZE) {
			i++;
			j = 0;
		}
	}

	array_new(ptr, ll);
	llnode_free(ll);

	(*ptr)->element_size = packet_get_elementsize(&(arr[0]));
}

bool packet_isnext(struct packet *a, struct packet *b)
{
	OPTIONAL_ASSERT(a != NULL);
	OPTIONAL_ASSERT(b != NULL);

	if (packet_get_datasum(a) != packet_get_datasum(b))
		return false;

	if (packet_get_elementsize(a) != packet_get_elementsize(b))
		return false;

	if (! (packet_get_index(a) + 1 == packet_get_index(b)))
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

	array_free(packets_get_packets(ptr));

	free(ptr);
}

struct array **packets_getptr_packets(struct packets *ptr)
{
	if (ptr == NULL)
		return NULL;

	return &(ptr->packets);
}

struct array *packets_get_packets(struct packets *ptr)
{
	if (ptr == NULL)
		return NULL;

	return ptr->packets;
}

void packets_pack(struct packets *ptr, struct array *src)
{
	OPTIONAL_ASSERT(ptr != NULL);
	pack(src, packets_getptr_packets(ptr));
}

void packets_unpack(struct packets *ptr, struct array **dst)
{
	OPTIONAL_ASSERT(ptr != NULL);
	unpack(packets_get_packets(ptr), dst);
}

void packets_send(struct packets *ptr, struct wopipe *pipe)
{
	OPTIONAL_ASSERT(ptr != NULL);
	wopipe_write(pipe, packets_get_packets(ptr));
}

void packets_receive(struct packets *ptr, struct ropipe *pipe)
{
	OPTIONAL_ASSERT(ptr != NULL);

	struct array *tmp = NULL;

	ropipe_read(pipe, &tmp, PACKET_SIZE, 1);

	struct llnode *ll = NULL;
	llnode_new(&ll, PACKET_SIZE, NULL);
	llnode_add(&ll, array_get(tmp, 0));
	array_free(tmp);
	tmp = NULL;

	struct packet *pkt = (struct packet *)llnode_get(ll, 0);

	if (pkt != NULL && packet_get_packetcount(pkt) > 0) {
		size_t packet_count = packet_get_packetcount(pkt);
		size_t remaining = 0;
		if (packet_count > 0)
			remaining = packet_count - 1;
		ropipe_read(pipe, &tmp, PACKET_SIZE, remaining);

		for (size_t i = packet_count - remaining, j = 0; i < packet_count; i++, j++) {
			llnode_add(&ll, array_get(tmp, j));
			struct packet *curr = (struct packet *)llnode_get(ll, i);
			struct packet *prev = (struct packet *)llnode_get(ll, i - 1);

			if (!(packet_isnext(prev, curr)))
				abort();
		}
		array_free(tmp);
	}

	array_new(packets_getptr_packets(ptr), ll);
	llnode_free(ll);
}