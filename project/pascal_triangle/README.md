# Pascal’s Triangle

A minimal C program that requests an integer `row ≥ 1` from the user and prints **Pascal’s Triangle** up to that row. It relies on a recursive factorial function to compute binomial coefficients $\binom{n}{k}$.

---

## 1. How It Works

```
Enter row: 5
           1
         1   1
       1   2   1
     1   3   3   1
   1   4   6   4   1
 1  5  10  10  5  1
```

### 1.1 Factorial (Recursive)

```c
long factorial(int n) {
    if (n == 0 || n == 1) return 1;
    return n * factorial(n - 1);
}
```

Simple but adequate for `n ≤ 12` (fits in 32‑bit signed long). For bigger triangles consider an iterative version with `long long` or `uint64_t`.

### 1.2 Binomial Coefficient

```c
long binomial(int n, int k) {
    return factorial(n) / (factorial(k) * factorial(n - k));
}
```

Computes $\binom{n}{k}$ using the factorial definition. Avoids overflow only for small rows.

### 1.3 Printing Loop

```c
for (int i = 0; i < row; ++i) {
    // leading spaces to center each row
    for (int j = 0; j < row - i; ++j) printf("  ");
    for (int j = 0; j <= i; ++j) printf("%4ld", binomial(i, j));
    putchar('\n');
}
```

* Centers text with `2*(row-i)` spaces left pad.
* `%4ld` gives fixed 4‑character cells so columns align nicely.

---

## 2. Build & Run

```bash
chmod +x build.sh
./build
./pascal
Enter row: 8
```

Sample output (rows 0–7):

```
               1
             1   1
           1   2   1
         1   3   3   1
       1   4   6   4   1
     1   5  10  10   5   1
   1   6  15  20  15   6   1
 1   7  21  35  35  21   7   1
```