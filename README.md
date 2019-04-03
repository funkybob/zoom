# zoom
Experiments with data compression


Each program builds on the main.c in the root directory, and provides a different implementation complying with the same interface:

extern void compress(FILE *in, FILE *out);
extern void decompress(FILE *in, FILE *out);

I plan to change this soon to:

extern void compress(uint8_t *src, size_t src_size, uint8_t *dest, size_t *dest_size);
extern void decompress(uint8_t *src, size_t src_size, uint8_t *dest, size_t *dest_size);

to remove file access from the code.

Perhaps also to use a struct as follows to hold the current encoder state:

struct {
    uint8_t *src;
    size_t ssrc;
    uint8_t *dest;
    size_t sdest;
}

Each encoder could expand on this struct to include whatever other state it requires.
