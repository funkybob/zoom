#include <stdint.h>
#include <unistd.h>

#define MAX_FRAME_SIZE (1<<22)

#define LAZY_PARSE 1

// #define DEBUG 1

#ifdef DEBUG
#define LOG(...)    { printf(__VA_ARGS__); fflush(stdout); }
#else
#define LOG(...)    /* __VA_ARGS__ */
#endif

struct buffer {
	uint32_t size;
	uint8_t *data;
};

uint32_t compress(struct buffer *src, struct buffer *dest);
uint32_t decompress(struct buffer *src, struct buffer *dest);
