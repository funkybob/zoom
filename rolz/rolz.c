// Reduced Offset LZ
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "comp.h"

#define FRAME_SIZE (1 << 24)

#define MAX_LIT_RUN 128     // implied 1
#define MAX_MATCH_LEN (255 + 3) // run of 0 == 3, run of 255 == 258

#define HASH_BITS 12
#define HASH_SIZE (1 << HASH_BITS)
#define HASH_MASK (HASH_SIZE - 1)

#define HASH_DEPTH 128

#define UNSET 0xffffffff

uint8_t table_next[HASH_SIZE];
uint32_t table[HASH_SIZE][HASH_DEPTH];

size_t hash_ptr;

static inline void init_hash_table() {
    memset(table_next, 0, sizeof(table_next));
    memset(table, -1, sizeof(table));
    hash_ptr = 0;
}

static inline unsigned int hash(uint8_t a, uint8_t b) {
    return (a << 4) ^ b;    // guaranteed 12 bits
}

static inline void update_hash_table(struct buffer *src, size_t ptr) {

    while((hash_ptr + 2) < ptr) {
        int key = hash(src->data[hash_ptr], src->data[hash_ptr+1]);
        int idx = table_next[key];
        table[key][idx] = hash_ptr + 2;

        idx += 1;
        if(idx == HASH_DEPTH) idx = 0;
        table_next[key] = idx;
        hash_ptr += 1;
    }

}

static inline int find_match_length(uint8_t *buffer, size_t left, size_t right, int max_len) {
    int len = 0;

    uint32_t c;

    uint32_t *l = (uint32_t*)&buffer[left];
    uint32_t *r = (uint32_t*)&buffer[right];

    max_len = (max_len > MAX_MATCH_LEN) ? MAX_MATCH_LEN : max_len;

    while(len < max_len && (c = *l++ ^ *r++) == 0) {
        len += 4;
    }

    if(c != 0) {
        len += __builtin_ctz(c) >> 3;   // LITTLE ENDIAN!
    }

    return (len > max_len) ? max_len : len;
}

int lit_counter = 0;
uint8_t lit_pending[MAX_LIT_RUN];

static inline void flush_literals(struct buffer *dest) {
    if(lit_counter > 0) {
        dest->data[dest->size++] = lit_counter - 1;
        memcpy(&dest->data[dest->size], lit_pending, lit_counter);
        dest->size += lit_counter;
        lit_counter = 0;
    }
}

static inline void emit_literal(uint8_t lit, struct buffer *dest) {
    lit_pending[lit_counter++] = lit;
    if(lit_counter == MAX_LIT_RUN) {
        flush_literals(dest);
    }
}

static inline void emit_match(int idx, int len, struct buffer *dest) {
    flush_literals(dest);
    dest->data[dest->size++] = 0x80 | idx;
    dest->data[dest->size++] = len - 3;
}

struct buffer* compress(struct buffer *src) {

    struct buffer *dest = malloc(sizeof(struct buffer));
    dest->data = malloc(1 << 24);
    dest->size = 0;

    size_t ptr = 0;

    init_hash_table();

    emit_literal(src->data[ptr++], dest);
    emit_literal(src->data[ptr++], dest);

    while(ptr < src->size) {
        int key = hash(src->data[ptr-2], src->data[ptr-1]);

        int max_find_idx = -1, max_find_len = 2;    // matches under 2 bytes aren't worth it

        for(int idx = 0; idx < HASH_DEPTH; idx++) {
            int offset = table[key][idx];
            if(offset == UNSET) break;

            int len = find_match_length(src->data, offset, ptr, src->size - offset);
            if (len > max_find_len) {
                max_find_len = len;
                max_find_idx = idx;

                if (len == MAX_MATCH_LEN) break;
            }
        }
        if (max_find_idx != -1) {
            emit_match(max_find_idx, max_find_len, dest);
            ptr += max_find_len;
        } else {
            emit_literal(src->data[ptr++], dest);
        }
        update_hash_table(src, ptr);
    }
    flush_literals(dest);

    return dest;
}


struct buffer* decompress(struct buffer *src, size_t output_size) {

    struct buffer *dest = malloc(sizeof(struct buffer));
    dest->data = malloc(output_size);
    dest->size = 0;

    size_t sptr = 0;

    init_hash_table();

    while(sptr < src->size) {
        int c;
        c = src->data[sptr++];

        if(c & 0x80) { // match
            int idx = c & 0x7f;
            int len = src->data[sptr++] + 3;
            int key = hash(dest->data[dest->size - 2], dest->data[dest->size - 1]);
            int offset = table[key][idx];
            for(int x = 0; x < len; x++) {
                dest->data[dest->size++] = dest->data[offset + x];
            }
        } else {
            int len = c + 1;
            while(len--) {
                dest->data[dest->size++] = src->data[sptr++];
            }
        }
        update_hash_table(dest, dest->size);
    }

    return dest;
}
