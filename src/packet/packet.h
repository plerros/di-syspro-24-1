#ifndef SYSPROG24_1_PACKET_H
#define SYSPROG24_1_PACKET_H

#include <stddef.h>

#include "array.h"

struct packet
{
	size_t data_sum;
	size_t element_size; // of the source data
	size_t index;
	char data[PACKET_SIZE - (3 * sizeof(size_t))];
};

void pack(struct array *ptr, struct array **packets);
void unpack(struct array *packets, struct array **ptr);
bool packet_isnext(struct packet *a, struct packet *b);

#endif /*SYSPROG24_1_PACKET_H*/