Basic LZ77 style compressor.

- Uses hash table of last HASH_DEPTH offsets to test for match
- Encodes literals as 0lllllll
  + l is len-1 so can store 1 to 128 literals in a run
- Encodes matches as:
  + length - MinMatchLen
  + if l < 128: 1lll llll
  + else:  1lll llll llll llll
  + if offset < 0x8000: 0nnnnnnn nnnnnnnn
  + else:      1nnnnnnn nnnnnnnn nnnnnnnn
- Uses lazy matching

Reference: lz4 -BD -9 -k
File   | Compressed  | Time
enwik8 |  42,203,342 | ~11.5sec
enwik9 | 374,086,002 | ~1m31sec

Reference: ulz c9
File   | Compressed  | Time
enwik8 |  40,285,779 | ~58.1sec
enwik9 | 356,494,682 | ~8m3sec

Reference: gzip -9k
File   | Compressed  | Time
enwik8 |  36,445,248 | ~29.3sec
enwik9 | 322,591,995 | ~3m27sec

HASH_DEPTH = 32:

File   | Compressed  | Time
enwik8 |  41,881,611 | ~22sec
enwik9 | 365,990,954 | ~2m54sec

HASH_DEPTH = 64:

File   | Compressed  | Time
enwik8 |  40,720,380 | ~36.5sec
enwik9 | 355,971,487 | ~5m28sec

HASH_DEPTH = 128

File   | Compressed  | Time
enwik8 |  39,810,526 | ~57.4sec
enwik9 | 348,270,975 | ~8m53sec
