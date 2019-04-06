Reduced-Offset LZ

This version maintains a hash table if lists of "places of interest".

At any point, the compressor:

1. hashes the previous 2 bytes
2. scans the list of (128) entries in the hash table at that point for the
   longest match.
3. If no match is found, output a no-match flag and a literal.
4. If a match is found, output a match-flag, the index of the entry in the hash
   table list, and 1-byte length (-3)

Sequential literals are gathered in blocks of up to 128, and output together.
The length is encoded -1 (as no run would be output for 0-length).

The hash table and offset lists sizes can be tuned by #defines, but the current
encoding and hash function are written for these exact values.

Results:

File   | Compressed  | Time       | Decompress Time
enwik8 |  53,077,086 | ~50.8sec   | ~3sec
enwik9 | 462,694,065 | ~7m18.8sec | ~20.6sec

Timing performed on my i7-7500U laptop.

A faster match length function has been added, which reduces compression time
for enwik8 from over 50 seconds, to about 34sec.

It works by scanning the buffer 32bits at a time, and XOR-ing the values
together.  If they match, the result will be 0, and we continue.  If not, we
count the trailing zero bits in the value (trailing because x86 is
little-endian), and work out the first byte that didn't match (which is just
the bit position / 8)

File   | Compressed  | Time
enwik8 |  53,077,110 | ~32.7sec
enwik9 | 462,694,065 | ~5m3.8sec
