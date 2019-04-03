Reduced-Offset LZ

This version maintains a hash table if lists of "places of interest".

At any point, the compressor:
1. hashes the previous 2 bytes
2. scans the list of (128) entries in the hash table at that point for the longest match
3. If no match is found, output a no-match flag and a literal.
4. If a match is found, output a match-flag, the index of the entry in the hash table list, and 1-byte length (-3)

Sequential literals are gathered in blocks of up to 128, and output together. The length is encoded -1 (as no run would be output for 0-length)

The hash table and offset lists sizes can be tuned by #defines, but the current encoding and hash function are written for these exact values.

The code currently does many file accesses when compressing, so could be sped considerably if it used an output buffer.

Results:

File   | Compressed  | Time       | Decompress Time
enwik8 |  53,077,086 | ~50.8sec   | ~3sec
enwik9 | 462,694,065 | ~7m18.8sec | ~20.6sec

Timing performed on my i7-7500U laptop.
