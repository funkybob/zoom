
Differences from rolz/

- Min match length 2
- Max match length (255 + 7 + MIN_MATCH_LEN)
- Hash Bits 16
- Hash Depth 16
- Shifts whole history list, instead of cyclic update
- 8-byte at a time match function
- Encodes (0lllllll) for literal run
- Encodes (1llliiii) for match
  + if lll == 7, adds next byte to length


File   | Compressed  | Time
enwik8 |  51,391,141 | ~11.3sec
enwik9 | 449,420,341 | ~1m39sec

The shorter HASH DEPTH results in drastically faster compression, but loses some compression rate.
If we double the depth, we also lose a bit for length encoding.
