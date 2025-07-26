# Collatz Conjecture

Given an integer **n**, this program repeatedly applies the Collatz rules.

* **even** → `n = n / 2`
* **odd**  → `n = 3*n + 1`

— until it reaches 1, printing every intermediate value and the iteration count.

---

## 1. Build & Run

```bash
cc collatz_conjecture.c -o collatz_conjecture
./collatz_conjecture 37
```

If you forget the argument:

```
Usage: ./collatz_conjecture <number>
```

---

## 2. What the Code Does (Step by Step)

### 2.1 Parse Input

```c
if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
}
long long number = atoi(argv[1]);
```

* Requires exactly one numeric argument.
* Uses `atoi` (simple but no error checking); stores in `long long` so you can try fairly large values.

### 2.2 Main Loop

```c
int counter = 0;
while (number != 1) {
    if (number % 2 == 0)      number /= 2;     // even
    else                      number = 3*number + 1; // odd

    counter++;
    printf("Iteration: %d\nNumber: %lld\n--------------------\n", counter, number);
}
```

* Iterates until we hit 1.
* Tracks how many steps (`counter`).
* Prints each step in a friendly format.

---

## 3. Collatz Background (aka 3n+1 Problem)

For any positive integer *n*, repeat:

1. If *n* is even, divide by 2.
2. If *n* is odd, compute 3\* n \* + 1.

Conjecture: **all** starting values eventually reach 1. It’s unproven (open since 1937), but verified for huge ranges numerically.