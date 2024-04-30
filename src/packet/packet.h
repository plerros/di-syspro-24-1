#include <stddef.h>

#include "array.h"

void pack(struct array *ptr, struct array **packets);
void unpack(struct array *packets, struct array **ptr);
