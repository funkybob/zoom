
1. fixed encoding size
2. max match len MML + 0x7f

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
enwik8 |  45,877,684 | ~14.6sec
enwik9 | 405,815,904 | ~1m58sec
```

```
Lazy 1-step:
File   | Compressed  | Time
enwik8 |  45,966,409 | ~14.1sec
enwik9 | 406,685,070 | ~1m53sec
```


```
| comp     | lz4      | raw      |
+----------+----------+----------+---------
|  4780108 |  4432858 | 10192446 | dickens
| 24273638 | 22078922 | 51220480 | mozilla
|  4612719 |  4245256 |  9970564 | mr
|  3688949 |  3673865 | 33553445 | nci
|  3983287 |  3543791 |  6152192 | ooffice
|  4016048 |  3977550 | 10085684 | osdb
|  2256776 |  2111124 |  6627202 | reymont
|  6426585 |  6139569 | 21606400 | samba
|  5927378 |  5735283 |  7251944 | sao
| 14893424 | 14001560 | 41458703 | webster
|  7612899 |  7175041 |  8474240 | x-ray
|   806414 |   770079 |  5345280 | xml
```
