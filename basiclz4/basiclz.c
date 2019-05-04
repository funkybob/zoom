#include <assert.h>
#include <string.h>

#include <stdio.h> // printf

#include "comp.h"
#include "basiclz.h"

#define MAX_LIT_RUN 0x80    // implied 1
#define MIN_MATCH_LEN 3     // match encoding is L + 2O
#define MAX_MATCH_LEN (0x7f + MIN_MATCH_LEN)
#define MAX_OFFSET 0xffff

#define UNSET 0xffffffff

struct match {
    uint16_t offset;
    uint16_t len;
    uint8_t match:1;
};

// track where we've updated the hash table to
uint32_t update_ptr;

uint32_t heads[1<<16];  // chain heads
uint32_t links[MAX_FRAME_SIZE];  // chain links: key is position

static inline void init_links_table(void) {
    memset(heads, -1, sizeof(heads));
    memset(links, -1, sizeof(links));
    update_ptr = 0;
}

static inline int build_key(uint8_t *src) {
    return ( *(uint32_t*)src * 123456791 ) >> 16;
}

static inline void update_links_table(struct buffer *src, uint32_t ptr) {
    while(update_ptr < (ptr - 4)) {
        int key = build_key(&src->data[update_ptr]);
        uint32_t position = heads[key];
        if(position != UNSET) {
            links[update_ptr] = position;
        }
        heads[key] = update_ptr;
        update_ptr += 1;
    }
}

static inline void emit_literal_run(uint8_t *src, uint32_t len, struct buffer *dest) {
    while(len > 0) {
        uint32_t size = (len > MAX_LIT_RUN) ? MAX_LIT_RUN : len;
        len -= size;
        dest->data[dest->size++] = (size - 1);

        while(size--) {
            dest->data[dest->size++] = *src++;
        }
    }
}

// offset is the absolute distance back we need to look
static inline void emit_match(uint16_t offset, int16_t len, struct buffer *dest) {
    len -= MIN_MATCH_LEN;
    dest->data[dest->size++] = 0x80 | len;

    *(uint16_t*)&dest->data[dest->size] = offset;
    dest->size += 2;
}

static inline uint16_t find_match_length(uint8_t *left, uint8_t* right, uint32_t max_len) {
    uint16_t len = 0;

    uint64_t c = 0;

    uint64_t *l = (uint64_t*)left;
    uint64_t *r = (uint64_t*)right;

    while(len < max_len && (c = *l++ ^ *r++) == 0) {
        len += 8;
    }

    if(c != 0) {
        len += __builtin_ctzl(c) >> 3;   // LITTLE ENDIAN!
    }

    return (len > max_len) ? max_len : len;
}

static inline void find_match(struct match *m, struct buffer *src, uint32_t ptr) {

    m->match = 0;
    m->offset = 0;
    m->len = 0;

    int key = build_key(&src->data[ptr]);
    uint32_t position = heads[key];

    while(position != UNSET) {
        if (ptr - position > MAX_OFFSET) break;

        uint32_t max_len = src->size - position;
        max_len = (max_len > MAX_MATCH_LEN) ? MAX_MATCH_LEN : max_len;

        uint16_t len = find_match_length(&src->data[position], &src->data[ptr], max_len);

        if (len > MIN_MATCH_LEN) {
            if (len > m->len) {
                m->match = 1;
                m->len = len;
                m->offset = ptr - position;

                if (len == max_len) {
                    break;
                }
            }
        }

        position = links[position];
    }

}

uint32_t compress(struct buffer *src, struct buffer *dest) {
    uint32_t ptr = 4;
    uint32_t head = ptr;

    dest->size = 0;

    init_links_table();

    emit_literal_run(src->data, 4, dest);
    update_links_table(src, ptr);

    struct match here;

#ifdef LAZY_PARSE
    struct match next;

    next.match = 0;
    next.len = 0;
    next.offset = 0;
#endif

    while(ptr < (src->size - 7)) {
#ifdef LAZY_PARSE
        if (next.match == 0) {
            find_match(&here, src, ptr);
        } else {
            here.match = 1;
            here.len = next.len;
            here.offset = next.offset;
        }
#else
        find_match(&here, src, ptr);
#endif

        if (here.match) {
#ifdef LAZY_PARSE
            uint8_t lit_bias = (head != ptr) ? 0 : 1;

            ptr += 1;
            update_links_table(src, ptr);
            find_match(&next, src, ptr);

            if (
                // is it a match?
                !next.match ||
                // will it result in a better yield?
                ((next.len - lit_bias) <= here.len)
            ) {
                ptr -= 1;
                if(ptr != head) {
                    emit_literal_run(&src->data[head], ptr - head, dest);
                }
                emit_match(here.offset, here.len, dest);
                ptr += here.len;
                head = ptr;
                next.match = 0;
            }
#else
            if(ptr != head) {
                emit_literal_run(&src->data[head], ptr - head, dest);
            }
            emit_match(here.offset, here.len, dest);
            ptr += here.len;
            head = ptr;
#endif
        } else {
            ptr += 1;
        }
        update_links_table(src, ptr);
    }

    if(head < src->size) {
        emit_literal_run(&src->data[head], src->size - head, dest);
    }

    return dest->size;
}

uint32_t decompress(struct buffer *src, struct buffer *dest, uint32_t output_size) {

    dest->size = 0;

    uint32_t sptr = 0;

    while(sptr < src->size) {
        unsigned int c;
        c = src->data[sptr++];

        if(c & 0x80) { // match
            unsigned int len = c & 0x7f;
            len += MIN_MATCH_LEN;

            uint16_t offset = *(uint16_t *)&src->data[sptr];
            sptr += 2;

            uint32_t position = dest->size - offset;

            assert(position < dest->size);

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
