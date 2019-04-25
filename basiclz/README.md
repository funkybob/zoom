Basic LZ77 style compressor.

- Uses hash table of last N offsets to test for match
- Encodes literals as 0lllllll
  + l is len-1 so can store 1 to 128 literals in a run
- Encodes matches as:
  + length - MinMatchLen
  + length: 1lllllll, if l == 127, output another byte
  + offset: 0nnnnnnn nnnnnnnn
  + or    : 1nnnnnnn nnnnnnnn nnnnnnnn
- Uses lazy matching

Reference: lz4 -BD -9 -k
File   | Compressed  | Time
enwik8 |  42,203,342 | ~11.5sec
enwik9 | 374,086,002 | ~1m31sec

Hash depth 64:

File   | Compressed  | Time
enwik8 |  43,306,021 | ~30.2sec
enwik9 | 377,887,729 | ~4m35sec

Hash depth 128:

File   | Compressed  | Time
enwik8 |  41,975,794 | ~54.5sec
enwik9 | 366,172,250 | ~9m16sec

Observations and future improvements:

- Better compression through larger choices
- Slower compression, because of more tests
- Discards known match for lazy matching!!
