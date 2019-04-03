// Reduced Offset LZ
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

uint8_t buffer[FRAME_SIZE];

size_t hash_ptr;

void init_hash_table() {
    memset(table_next, 0, sizeof(table_next));
    memset(table, -1, sizeof(table));
    hash_ptr = 0;
}

unsigned int hash(uint8_t a, uint8_t b) {
    return (a << 4) ^ b;    // guaranteed 12 bits
}

void update_hash_table(size_t ptr) {

    while((hash_ptr + 2) < ptr) {
        int key = hash(buffer[hash_ptr], buffer[hash_ptr+1]);
        int idx = table_next[key];
        table[key][idx] = hash_ptr + 2;

        idx += 1;
        if(idx == HASH_DEPTH) idx = 0;
        table_next[key] = idx;
        hash_ptr += 1;
    }

}

int find_match_length(uint8_t *buffer, size_t left, size_t right, int max_len) {
    int len = 0;
    while(len < max_len && len < MAX_MATCH_LEN) {
        if (buffer[left + len] != buffer[right + len]) break;
        len += 1;
    }
    return len;
}

int lit_counter = 0;
uint8_t lit_pending[MAX_LIT_RUN];

void flush_literals(FILE *fout) {
    if(lit_counter > 0) {
        fputc(lit_counter-1, fout);
        fwrite(lit_pending, 1, lit_counter, fout);
        lit_counter = 0;
    }
}

void emit_literal(uint8_t lit, FILE *fout) {
    lit_pending[lit_counter++] = lit;
    if(lit_counter == MAX_LIT_RUN) {
        flush_literals(fout);
    }
}

void emit_match(int idx, int len, FILE *fout) {
    flush_literals(fout);
    fputc(0x80 | idx, fout);
    fputc(len-3, fout);
}

void compress(FILE *fin, FILE *fout) {

    while(1) {
        size_t ptr = 0;
        size_t size = fread(buffer, 1, FRAME_SIZE, fin);
        if(size == 0) break;

        init_hash_table();

        fwrite(&size, 4, 1, fout);

        emit_literal(buffer[ptr++], fout);
        emit_literal(buffer[ptr++], fout);

        while(ptr < size) {
            int key = hash(buffer[ptr-2], buffer[ptr-1]);

            int max_find_idx = -1, max_find_len = 2;    // matches under 2 bytes aren't worth it

            for(int idx = 0; idx < HASH_DEPTH; idx++) {
                int offset = table[key][idx];
                if(offset == UNSET) break;

                int len = find_match_length(buffer, offset, ptr, size - offset);
                if (len > max_find_len) {
                    max_find_len = len;
                    max_find_idx = idx;

                    if (len == MAX_MATCH_LEN) break;
                }
            }
            if (max_find_idx != -1) {
                emit_match(max_find_idx, max_find_len, fout);
                ptr += max_find_len;
            } else {
                emit_literal(buffer[ptr++], fout);
            }
            update_hash_table(ptr);
        }
        flush_literals(fout);
    }

}


void decompress(FILE *fin, FILE *fout) {

    while(1) {
        size_t size, ptr;
        ptr = fread(&size, 1, 4, fin);
        if(ptr == 0) break;

        init_hash_table();

        ptr = 0;
        while(ptr < size) {
            int c;
            c = fgetc(fin);
            if(c & 0x80) { // match
                int idx = c & 0x7f;
                int len = fgetc(fin) + 3;
                int key = hash(buffer[ptr-2], buffer[ptr-1]);
                int offset = table[key][idx];
                for(int x = 0; x < len; x++) {
                    buffer[ptr+x] = buffer[offset+x];
                }
                ptr += len;
            } else {
                int len = c + 1;
                fread(&buffer[ptr], 1, len, fin);
                ptr += len;
            }
            update_hash_table(ptr);
        }
        fwrite(buffer, 1, size, fout);
    }
}
