# Happy Number Checker

A tiny command‑line utility that determines whether a given integer is a **happy number** using Floyd’s cycle‑finding algorithm (the “tortoise and hare” technique). No external libraries—just the C standard library.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build
./happy_numbers 19   # prints that 19 is happy
./happy_numbers 20   # prints that 20 is not happy
```

Pass a single positive integer as argument. If omitted, the program prints a usage line.

---

## 2. What’s a Happy Number?

A positive integer is *happy* if, after repeatedly replacing the number by the sum of the squares of its digits, you eventually reach **1**. If you fall into a cycle that never reaches 1, the number is *unhappy*.

Example for 19:

1. 1² + 9² = 82
2. 8² + 2² = 68
3. 6² + 8² = 100
4. 1² + 0² + 0² = 1 → **happy!**

---

## 3. Core Functions

### 3.1 `sum_squared_digits()`

```c
int sum_squared_digits(int n) {
    int sum = 0;
    while (n > 0) {
        int d = n % 10;   // last digit
        sum += d * d;     // add square
        n /= 10;          // drop last digit
    }
    return sum;
}
```

Computes the “next” number in the happy‑number sequence.

### 3.2 Main Algorithm (Floyd’s Cycle Detection)

```c
int slow = number;                   // tortoise: one step each loop
int fast = sum_squared_digits(number); // hare: two steps each loop

while (fast != 1 && slow != fast) {
    slow = sum_squared_digits(slow);
    fast = sum_squared_digits(sum_squared_digits(fast));
}
```

* If `fast` hits 1 → the sequence reaches 1 → happy.
* If `slow == fast` before 1 → we’re in a cycle that excludes 1 → unhappy.

Using two pointers avoids storing a set of seen values; memory cost is O(1).

---

## 4. Output

* **Happy:** `CONGRATULATION, <n> IS A HAPPY NUMBER!`
* **Unhappy:** `SORRY, <n> IS NOT A HAPPY NUMBER.`

---

## 5. Complexity

* **Time:** O(k·log₁₀ n) where *k* is the number of iterations until cycle detection—bounded because sums shrink quickly.
* **Memory:** O(1) thanks to Floyd’s algorithm.