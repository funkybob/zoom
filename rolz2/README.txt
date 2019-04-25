
Differences from rolz/

- Min match length 2
- Max match length (255 + 7 + MIN_MATCH_LEN)
- Hash Bits 16
- Hash Depth 16
- CRC32 based hash function
- Shifts whole history list, instead of cyclic update
- Encodes (0lllllll) for literal run
- Encodes (1llliiii) for match
  + if lll == 7, adds next byte to length
- Lazy parsing will skip match if next position has larger match
  + compensates for creating new literal run


File   | Compressed  | Time
enwik8 |  50,513,318 | ~15.7sec
enwik9 | 441,232,486 | ~2m14sec

The shorter HASH DEPTH results in drastically faster compression, but loses some compression rate.
If we double the depth, we also lose a bit for length encoding.
