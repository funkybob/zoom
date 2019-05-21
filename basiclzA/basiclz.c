// Reduced Offset LZ
#include <assert.h>
#include <string.h>

#include "comp.h"

#define MAX_LIT_RUN 128     // implied 1
#define MIN_MATCH_LEN 4     // minimum match encoding is L + 2O
#define MAX_MATCH_LEN ((1<<14) + MIN_MATCH_LEN -1)

#define HASH_BITS 16
#define HASH_SIZE (1 << HASH_BITS)
#define HASH_MASK (HASH_SIZE - 1)

#define HASH_DEPTH 128

#define UNSET 0xffffffff

uint32_t table[HASH_SIZE][HASH_DEPTH];
uint8_t table_start[HASH_SIZE];

uint32_t update_ptr;

struct match {
    uint32_t offset;
    uint16_t len;
    uint8_t cost;
};

static inline void init_hash_table() {
    memset(table, -1, sizeof(table));
    memset(table_start, 0, sizeof(table_start));
    update_ptr = 0;
}

static inline int hash(uint8_t *src) {
    return ( *(uint32_t*)src * 123456791 ) >> 16;
}

static inline void update_hash_table(struct buffer *src, uint32_t ptr) {

    while(update_ptr < ptr) {
        uint32_t key = hash(&src->data[update_ptr]);

        table[key][table_start[key]] = update_ptr;
        table_start[key] = (table_start[key] + 1) % HASH_DEPTH;

        update_ptr += 1;
    }

}

static inline void emit_literal_run(uint8_t *src, uint32_t len, struct buffer *dest) {
    while(len > 0) {
        uint32_t size = (len > MAX_LIT_RUN) ? MAX_LIT_RUN : len;
        dest->data[dest->size++] = (size - 1);

        len -= size;
        while(size--) {
            dest->data[dest->size++] = *src++;
        }
    }
}

// offset is the absolute distance back we need to look
static inline void emit_match(uint32_t offset, int len, struct buffer *dest) {
    len -= MIN_MATCH_LEN;

    if ( len < 0x3f ) {
        dest->data[dest->size++] = 0x80 | len;
    } else {
        dest->data[dest->size++] = 0xc0 | (len >> 8);
        dest->data[dest->size++] = len & 0xff;
    }

    if(offset < 0x8000) {
        dest->data[dest->size++] = offset >> 8;
        dest->data[dest->size++] = offset & 0xff;
    } else {
        dest->data[dest->size++] = 0x80 | (offset >> 16);
        dest->data[dest->size++] = offset >> 8;
        dest->data[dest->size++] = offset & 0xff;
    }
}

static inline uint8_t encoding_size(uint32_t len, size_t offset) {
    size_t l = 0;

    len -= MIN_MATCH_LEN;

    l += (len > 0x3f) ? 2 : 1;

    l += (offset > 0x7fff) ? 3 : 2;

    return l;
}

static inline size_t find_match_length(uint8_t *buffer, size_t left, size_t right, size_t max_len) {
    size_t len = 0;

    uint64_t c = 0;

    uint64_t *l = (uint64_t*)&buffer[left];
    uint64_t *r = (uint64_t*)&buffer[right];

    max_len = (max_len > MAX_MATCH_LEN) ? MAX_MATCH_LEN : max_len;

    while(len < max_len && (c = *l++ ^ *r++) == 0) {
        len += 8;
    }

    if(c != 0) {
        len += __builtin_ctzll(c) >> 3;   // LITTLE ENDIAN!
    }

    return (len > max_len) ? max_len : len;
}

static inline void find_match(struct match *m, struct buffer *src, size_t ptr) {
    uint32_t key = hash(&src->data[ptr]);

    uint16_t best_saving = 0;

    m->offset = 0;
    m->len = 0;
    m->cost = 0;

    for (int idx = 0; idx < HASH_DEPTH; idx++) {
        uint32_t position = table[key][idx];
        if(position == UNSET) break;

        uint16_t len = find_match_length(src->data, position, ptr, src->size - position);

        if (len >= MIN_MATCH_LEN) {
            uint8_t cost = encoding_size(len, ptr - position);

            if ((len - cost) > best_saving) {
                best_saving = len - cost;
                m->len = len;
                m->offset = ptr - position;
                m->cost = cost;
            }
        }
    }
}

uint32_t compress(struct buffer *src, struct buffer *dest) {
    uint32_t head = 0;
    uint32_t ptr = 0;

    dest->size = 0;

    init_hash_table();

    struct match here, next;

    next.len = 0xffff;

    while(ptr < (src->size - 7)) {
        if (next.len == 0xffff) {
            find_match(&here, src, ptr);
        } else {
            here.len = next.len;
            here.offset = next.offset;
            here.cost = next.cost;
        }

        if (here.len > here.cost) {

            // will emitting a literal now incurr a literal block cost?
            uint8_t lit_bias = (head == ptr) ? 1 : 0;

            ptr += 1;
            update_hash_table(src, ptr);
            find_match(&next, src, ptr);

            if (
                // is it a match?
                (next.len <= next.cost) ||
                // will it result in a better yield?
                ((next.len - (next.cost + lit_bias)) <= (here.len - here.cost))
            ) {
                ptr -= 1;
                emit_literal_run(&src->data[head], ptr - head, dest);
                emit_match(here.offset, here.len, dest);
                ptr += here.len;
                head = ptr;
                next.len = 0xffff;
            }
        } else {
            ptr += 1;
        }
        update_hash_table(src, ptr);
    }

    if(ptr < src->size)
        emit_literal_run(&src->data[ptr], src->size- head, dest);

    return dest->size;
}


uint32_t decompress(struct buffer *src, struct buffer *dest, uint32_t output_size) {

    dest->size = 0;

    size_t sptr = 0;

    while(sptr < src->size) {
        int c;
        c = src->data[sptr++];

        if(c & 0x80) { // match
            int len = c & 0x3f;
            if(c & 0x40) {
                len = (len << 8) | src->data[sptr++];
            }
            len += MIN_MATCH_LEN;

            c = src->data[sptr++];
            size_t offset = c & 0x7f;
            offset = (offset << 8) | src->data[sptr++];
            if(c & 0x80) {
                offset = (offset << 8) | src->data[sptr++];
            }

            uint32_t position = dest->size - offset;
            while(len--)
                dest->data[dest->size++] = dest->data[position++];
        } else {
            int len = c + 1;
            while(len--) {
                dest->data[dest->size++] = src->data[sptr++];
            }
        }
    }

    assert(dest->size == output_size);

    return dest->size;
}
