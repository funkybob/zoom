#ifndef __COMP_H__

#include <stdint.h>
#include <unistd.h>

struct buffer {
	size_t size;
	uint8_t *data;
};

extern struct buffer* compress(struct buffer *src);
extern struct buffer* decompress(struct buffer *src, size_t size);

#endif
