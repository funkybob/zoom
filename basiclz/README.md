Basic LZ77 style compressor.

- Uses hash table of last HASH_DEPTH offsets to test for match
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

HASH_DEPTH = 64:

File   | Compressed  | Time
enwik8 |  43,305,931 | ~29.8sec
enwik9 | 377,883,432 | ~4m22sec

HASH_DEPTH = 128:

File   | Compressed  | Time
enwik8 |  41,975,759 | ~49.8sec
enwik9 | 366,172,250 | ~8m55sec

Observations and future improvements:

- Better compression through larger choices
- Slower compression, because of more tests

On enwik8, at HASH_DEPTH = 64 reusing the lazy lookahead matches saved ~100,000 calls to find_match per 8MB block.
However, that still left ~2.1million.
At HASH_DEPTH = 128 it has a more significant impact, reducing enwik8 from >60sec to ~49sec, and enwik9 from ~9m16sec to ~8m55sec.
