# Generating Primes up to 1e8

## Compile and run:
```bash
g++ hw1.cpp -O3 -o run
./run```

We use a naive O(N * sqrt(N)) brute-force primality check for each number between 1 and 1e8. We split up the range to check into 8 segments of equal length (although smaller numbers will take on average less time to check,
it might be more favorable to skew these ranges so the first few have a larger space to work with, whatever equally distributes the primes within the ranges).

It's worth noting that a sieve approach (O(nloglogn)) outperforms the brute force, even with 8 concurrent threads (all used).

The primality check only returns true when a number n is not divisible by any number at most its sqrt (or n is 2).
Since any divisor of a number is at most the sqrt of that number, the primality check is correct.
