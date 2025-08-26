#!/usr/bin/env python3
"""Recursive Fibonacci sequence."""

import time

def rfib(n):
    if n < 1:
        return 0
    if n == 1:
        return 1
    return rfib(n-1) + rfib(n-2)

def run_benchmark(n=40):
    t0 = time.time()
    x = rfib(n)
    t1 = time.time()
    elapsed = t1 - t0
    print(f"rfib({n}) = {x}, time: {elapsed:.3f} seconds")
    results[f"rfib({n})"] = round(elapsed, 3)

if __name__ == "__main__":
    results = {}
    run_benchmark()