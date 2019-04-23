#ifndef __COMP_H__

#include <stdint.h>
#include <unistd.h>

struct buffer {
	size_t size;
	uint8_t *data;
};

size_t compress(struct buffer *src, struct buffer *dest);
size_t decompress(struct buffer *src, struct buffer *dest, size_t size);

#endif
