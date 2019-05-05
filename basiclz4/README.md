
1. fixed encoding size
2. max match len MML + 0x7f

"Greedy" parser takes longest match it can find.

"Lazy" parser will check if matching on the next byte would be better. If it is, it starts over, emitting the current byte as a literal. This is unbounded in how many skips it may result in, so it's possible a short match will be passed over in the literals entirely, on the way to the winning longer match.

```
Reference: lz4 -BD -9k
File   | Compressed  | Time
enwik8 |  42,203,342 | ~11.5sec
enwik9 | 374,086,002 | ~1m31sec
```

```
Reference: lz4 -9k
File   | Compressed  | Time
enwik8 |  42,276,858 | ~11.5sec
enwik9 | 374,839,215 | ~1m29sec
```

```
Reference: ulz c9
File   | Compressed  | Time
enwik8 |  40,285,779 | ~58.1sec
enwik9 | 356,494,682 | ~8m3sec
```

```
Reference: gzip -9k
File   | Compressed  | Time
enwik8 |  36,445,248 | ~29.3sec
enwik9 | 322,591,995 | ~3m27sec
```

```
Greedy:
File   | Compressed  | Time
enwik8 |  46,592,419 | ~9.8s
enwik9 | 412,564,110 | ~1m24sec
```

```
Lazy:
File   | Compressed  | Time
enwik8 |  45,877,500 | ~14.6sec
enwik9 | 405,807,570 | ~2m11sec
```

```
Lazy 1-step:
File   | Compressed  | Time
enwik8 |  45,966,409 | ~14.1sec
enwik9 | 406,685,070 | ~1m53sec
```

Silesia corups

```
| greedy   | lazy     | lz4      | raw      |
+----------+----------+----------+----------+---------
|  4825466 |  4780099 |  4432858 | 10192446 | dickens
| 24819672 | 24273074 | 22078922 | 51220480 | mozilla
|  4639408 |  4612166 |  4245256 |  9970564 | mr
|  3779120 |  3688097 |  3673865 | 33553445 | nci
|  4076538 |  3983258 |  3543791 |  6152192 | ooffice
|  4177378 |  4016045 |  3977550 | 10085684 | osdb
|  2273417 |  2256774 |  2111124 |  6627202 | reymont
|  6479731 |  6425837 |  6139569 | 21606400 | samba
|  6053921 |  5927376 |  5735283 |  7251944 | sao
| 15116442 | 14893412 | 14001560 | 41458703 | webster
|  7633109 |  7612896 |  7175041 |  8474240 | x-ray
|   823673 |   806228 |   770079 |  5345280 | xml
```

From some analysis, it appears two big advantages in LZ4 are:

1. the lack of explicit match/literal flags.

   instead, encoding assumes it will always be (literal run, match) and wears a small penalty (4bits) when it's wrong.

2. more efficient encoding of short runs

   by allowing literal run and match length encoding to use only 4bits for runs up to 16, more savings are seen. Some simple analysis shows that by far the most common lengths are under this size.
