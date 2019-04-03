Simple predictor compression.

Encoding:
1. initialise hash table to 0
2. read byte
3. hash byte with current hash key
4. if hash_table[key] == byte, emit match flag
5. else, emit no-match flag, and literal byte
6. update hash_table[key] = byte
7. repeat to end of file.

To avoid dealing with non-byte aligned writes, we gather up literals until we've hit 8 no/match flags.

Compression can be varied by changing the hash table size and the hashing algorithm.

Default hash is simply (key << 4) ^ c

Table Size | Result (enwik8)
16MB       | 55448062 B
 1MB       | 57281519 B
