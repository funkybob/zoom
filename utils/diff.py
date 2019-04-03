#!/usr/bin/env python3

import sys


def stream(fn):
    with open(fn) as fin:
        while True:
            l = fin.readline()
            if not l:
                break
            yield l


line = 0
output = 0

l = stream(sys.argv[1])
r = stream(sys.argv[2])

q = []

while True:
    left = next(l).strip()
    right = next(r).strip()
    if left != right:
        if not output:
            for ln, lx in q:
                print(f'|{ln:>6d}| {lx}')
        print(f'<{line:>6d}< {left}')
        print(f'>{line:>6d}> {right}')
        output = 5
    elif output:
        print(f'|{line:>6d}| {left}')
        output -= 1

    q.append((line, left))
    while(len(q) > 5):
        q.pop(0)

    line += 1
