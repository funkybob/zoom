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
enwik8 |  42,914,828 | ~38.8sec
enwik9 | 374,719,846 | ~5m42sec

HASH_DEPTH = 128:

File   | Compressed  | Time
enwik8 |  41,486,119 | ~1m9sec
enwik9 | 362,226,644 | ~10m7sec

Observations and future improvements:

- Better compression through larger choices
- Slower compression, because of more tests

On enwik8, at HASH_DEPTH = 64 reusing the lazy lookahead matches saved ~100,000 calls to find_match per 8MB block.
However, that still left ~2.1million.
At HASH_DEPTH = 128 it has a more significant impact, reducing enwik8 from >60sec to ~49sec, and enwik9 from ~9m16sec to ~8m55sec.

By moving the encoding cost considerations into the match finding loop we improve compression noticeably, at the cost of performance.
