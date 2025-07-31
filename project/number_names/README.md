# Number‑to‑Words Converter (0 – 9 999)

A compact C program that converts an integer in the range **0 – 9999** into its English words representation (e.g. `5421 → "five thousand four hundred twenty one"`). No external libraries—just the C standard library.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build
./number_names 1987   # prints: "one thousand nine hundred eighty seven"
```

If you omit the argument or pass a value outside `0–9999`, the program prints a usage / range error.

---

## 2. High‑Level Flow

1. **Parse CLI argument → int `n`.**
2. **Early exits** for bad range and the special case `0`.
3. Split `n` into:

   * `thousands = n / 1000`
   * `hundreds  = (n % 1000) / 100`
   * `tens_digit = (n % 100) / 10`
   * `units_digit = n % 10`
4. Build the output string piecemeal, handling the **11–19** special words.
5. Print `\n` at the end.

No dynamic allocations are needed; everything prints directly to stdout.

---

## 3. Word Tables

```c
char *units[] = { "zero", "one", "two", ..., "nine" };
char *eleven_nineteen[] = { "eleven", "twelve", ..., "nineteen" };
char *tens[] = { "ten", "twenty", ..., "ninety" };
```

* Indexing is direct (`units[3] -> "three"`).
* The program special‑cases `ten`, `11–19`, and `tens_digit ≥ 2`.

---

## 4. Printing Logic (Step‑by‑Step)

```c
if (thousands > 0) {
    printf("%s thousand", units[thousands]);
    if (hundreds || tens_digit || units_digit) printf(" ");
}
```

* Adds a space **only if** more chunks follow.

```c
if (hundreds > 0) {
    printf("%s hundred", units[hundreds]);
    if (tens_digit || units_digit) printf(" ");
}
```

### 4.1 11 – 19

```c
if (tens_digit == 1 && units_digit > 0) {
    printf("%s", eleven_nineteen[units_digit - 1]);
}
```

### 4.2 Exact "ten"

```c
else if (tens_digit == 1 && units_digit == 0) {
    printf("ten");
}
```

### 4.3 20 – 99

```c
else {
    if (tens_digit > 1) {
        printf("%s", tens[tens_digit - 1]);
        if (units_digit > 0) printf(" ");
    }
    if (units_digit > 0) {
        printf("%s", units[units_digit]);
    }
}
```

---

## 5. Input Helpers (Unused Now)

The file also includes two utilities that **are not used** in `main` but could be handy:

* `count_digits(int n)` – returns digit count (handles negative, zero).
* `get_digits(int n, int *len)` – allocates an `int*` array containing digits in order.

You can remove them to shrink the binary or repurpose them for future extensions.

---

## 6. Edge‑Case Checklist

* ✔ `0` prints "zero" and exits.
* ✔ `10` prints "ten" (special case).
* ✔ `11–19` print unique teen words.
* ✔ No trailing spaces.
* ✔ Up to `9999` inclusive.