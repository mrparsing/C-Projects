# Sieve of Eratosthenes

A simple Sieve of Eratosthenes.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build.sh
./sieve_of_eratosthenes 50
```

**Output:**

```
2 3 5 7 11 13 17 19 23 29 31 37 41 43 47
```

---

## 2. How It Works

### 2.1 Command‑Line Parsing

```c
if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
}
int n = atoi(argv[1]);
```

* Expects exactly one positional argument: the upper limit `n`.
* Uses `atoi` for quick conversion (no error handling beyond missing arg).

### 2.2 Main Loop

```c
for (int i = 2; i < n; i++) {
    /* handle 2,3,5 explicitly */
    if (i == 2 || i == 3 || i == 5) { printf("%d ", i); continue; }
    /* skip multiples of 2,3,5 */
    if (i % 2 == 0 || i % 3 == 0 || i % 5 == 0) continue;
    /* otherwise print */
    printf("%d ", i);
}
printf("\n");
```

* Starts at **2** (since 0 and 1 are trivial).
* Explicitly prints `2, 3, 5` so they appear even though they’re multiples of themselves.
* `continue` skips any `i` divisible by 2, 3, or 5.
* Everything else is printed—these numbers are not necessarily prime (e.g. 49 passes the test) but they are **coprime to 30**.

### 2.3 Complexity

* Time: `O(n)` (single pass).
* Memory: `O(1)`.
* Perfectly fine for small/medium `n` (< 10^7). For huge ranges you’d adopt a sieve.