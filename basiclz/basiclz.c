// Reduced Offset LZ
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "comp.h"

#define MAX_LIT_RUN 128     // implied 1
#define MIN_MATCH_LEN 4     // minimum match encoding is L + 2O
#define MAX_MATCH_LEN (255 + 127 + MIN_MATCH_LEN)

#define HASH_BITS 16
#define HASH_SIZE (1 << HASH_BITS)
#define HASH_MASK (HASH_SIZE - 1)

#define HASH_DEPTH 64

#define UNSET 0xffffffff

uint32_t table[HASH_SIZE][HASH_DEPTH];
uint32_t hash_ptr;

struct match {
    size_t offset;
    size_t len;
};

static inline void init_hash_table() {
    memset(table, -1, sizeof(table));
    hash_ptr = 0;
}

static inline uint32_t hash(uint8_t a, uint8_t b, uint8_t c) {
    uint32_t acc = (a<<16) | (b<<8) | c;
    acc = __builtin_ia32_crc32si(0, acc);
    return acc & HASH_MASK;
}

static inline void update_hash_table(struct buffer *src, size_t ptr) {

    while((hash_ptr + 3) < ptr) {
        uint32_t key = hash(src->data[hash_ptr], src->data[hash_ptr+1], src->data[hash_ptr+2]);
        memmove(&table[key][1], &table[key][0], sizeof(uint32_t) * (HASH_DEPTH-1));
        table[key][0] = hash_ptr;
        hash_ptr += 1;
    }

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
        len += __builtin_ctzl(c) >> 3;   // LITTLE ENDIAN!
    }

    return (len > max_len) ? max_len : len;
}

int lit_counter = 0;
uint8_t lit_pending[MAX_LIT_RUN];

static inline void flush_literals(struct buffer *dest) {
    if(lit_counter > 0) {
        dest->data[dest->size++] = lit_counter - 1;
        uint8_t *p = lit_pending;
        while(lit_counter--)
            dest->data[dest->size++] = *p++;
        lit_counter = 0;
    }
}

static inline void emit_literal(uint8_t lit, struct buffer *dest) {
    lit_pending[lit_counter++] = lit;
    if(lit_counter == MAX_LIT_RUN) {
        flush_literals(dest);
    }
}

// offset is the absolute distance back we need to look
static inline void emit_match(size_t offset, int len, struct buffer *dest) {
    flush_literals(dest);
    len -= MIN_MATCH_LEN;
    if ( len < 127 ) {
        dest->data[dest->size++] = 0x80 | len;
    } else {
        dest->data[dest->size++] = 0xff;
        dest->data[dest->size++] = len - 127;
    }

    // top bit encodes how many bytes
    // 0b0nnn nnnn nnnn nnnn
    // 0b1nnn nnnn nnnn nnnn nnnn nnnn
    // this limits out max offset to 7fffff
    if(offset < 0x8000) {
        dest->data[dest->size++] = 0x00 | (offset >> 8);
        dest->data[dest->size++] = offset & 0x00ff;
    } else {
        dest->data[dest->size++] = 0x80 | ((offset >> 16) & 0x00ff);
        dest->data[dest->size++] = (offset >> 8 ) & 0x00ff;
        dest->data[dest->size++] = offset & 0x00ff;
    }
}

static inline void find_match(struct match *m, struct buffer *src, size_t ptr) {
    uint32_t key = hash(src->data[ptr], src->data[ptr+1], src->data[ptr+2]);

    m->offset = 0;
    m->len = 0;

    for(int idx = 0; idx < HASH_DEPTH; idx++) {
        uint32_t offset = table[key][idx];
        if(offset == UNSET) break;

        size_t len = find_match_length(src->data, offset, ptr, src->size - offset);
        if (len > m->len) {
            m->len = len;
            m->offset = ptr - offset;

            if (len == MAX_MATCH_LEN) break;
        }
    }
}

static inline size_t encoding_size(struct match *m) {
    size_t l = 1;
    if(m->len > (126 + MIN_MATCH_LEN)) l += 1;

    l += 2;
    if(m->offset >= 0x8000) l += 1;

    return l;
}

size_t compress(struct buffer *src, struct buffer *dest) {
    dest->size = 0;

    size_t ptr = 0;

    init_hash_table();

    emit_literal(src->data[ptr++], dest);
    emit_literal(src->data[ptr++], dest);
    emit_literal(src->data[ptr++], dest);

    while(ptr < src->size) {
        struct match here;

        find_match(&here, src, ptr);

        size_t here_cost = encoding_size(&here);

        if (here.len > here_cost) {
            struct match next;

            ptr += 1;
            update_hash_table(src, ptr);
            find_match(&next, src, ptr);

            size_t next_cost = encoding_size(&next);

            // will emitting a literal now incurr a literal block cost?
            if(lit_counter == 0) next_cost += 1;

            if (
                (next.len > next_cost) &&
                ((next.len - next_cost) > (here.len - here_cost))
            ) {
                emit_literal(src->data[ptr-1], dest);
            } else {
                emit_match(here.offset, here.len, dest);
                ptr += here.len - 1;
            }
        } else {
            emit_literal(src->data[ptr++], dest);
        }
        update_hash_table(src, ptr);
    }
    flush_literals(dest);

    return dest->size;
}


size_t decompress(struct buffer *src, struct buffer *dest, size_t output_size) {

    dest->size = 0;

    size_t sptr = 0;

    while(sptr < src->size) {
        int c;
        c = src->data[sptr++];

        if(c & 0x80) { // match
            int len = c & 0x7f;
            if(len == 0x7f) {
                len += src->data[sptr++];
            }
            len += MIN_MATCH_LEN;
            c = src->data[sptr++];

            size_t offset = c & 0x7f;
            offset = (offset << 8) | src->data[sptr++];
            if(c & 0x80) {
                offset = (offset << 8) | src->data[sptr++];
            }

            offset = dest->size - offset;
            while(len--)
                dest->data[dest->size++] = dest->data[offset++];
        } else {
            int len = c + 1;
            while(len--) {
                dest->data[dest->size++] = src->data[sptr++];
            }
        }
        update_hash_table(dest, dest->size);
    }

    assert(dest->size == output_size);

    return dest->size;
}
