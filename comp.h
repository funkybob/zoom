#ifndef __COMP_H__
#define __COMP_H__

#include <stdint.h>
#include <unistd.h>

struct buffer {
	uint32_t size;
	uint8_t *data;
};

uint32_t compress(struct buffer *src, struct buffer *dest);
uint32_t decompress(struct buffer *src, struct buffer *dest, uint32_t size);

#endif
