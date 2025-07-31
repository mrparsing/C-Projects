# Credit Card Validator (Luhn Algorithm)

A tiny command‑line utility that checks whether a numeric string passes the **Luhn checksum** (used by most credit/debit cards, IMEI, etc.). Implementation is single‑file C, no external deps.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build
./validator 4532015112830366   # Visa test number → VALID
./validator 1234567812345670   # → NOT VALID
```

If you run without an argument:

```
Usage: ./validator <number>
```

---

## 2. How the Program Works

1. **Read input as a string** (up to 31 chars) so leading zeros are preserved and non‑digits can be skipped.
2. **Extract digits** into an `int digits[19]` array, counting how many were stored (`len`).
3. **Apply Luhn transform** starting from the **second‑to‑last** digit, moving left:

   * Double every second digit.
   * If the result exceeds 9, subtract 9 (equivalent to summing the two digits of the product).
4. **Sum all digits** (modified + untouched).
5. **Check mod 10**: if `sum % 10 == 0`, the number is valid.

---

## 3. Code Walkthrough (Key Blocks)

### 3.1 Input Capture & Digit Extraction

```c
char input[32];
int digits[19];
int len = 0;

scanf("%31s", input);
for (int i = 0; input[i] != '\0' && len < 19; i++) {
    if (isdigit((unsigned char)input[i])) {
        digits[len++] = input[i] - '0';
    }
}
```

* Keeps only digits, silently ignoring spaces or punctuation.
* Caps at 19 digits (longest mainstream PAN length). Adjust if needed.

### 3.2 Luhn Transformation Loop

```c
for (int i = len - 2; i >= 0; i -= 2) {
    int d = digits[i] * 2;
    if (d > 9) d -= 9;
    digits[i] = d;
}
```

* Starts at `len-2` (second from the right).
* Steps by 2 to process every other digit.
* `x*2 > 9 ? x*2-9 : x*2` is a fast way to sum the digits of the doubled number.

### 3.3 Summation & Verdict

```c
int sum = 0;
for (int i = 0; i < len; i++) sum += digits[i];

if (sum % 10 == 0) puts("VALID NUMBER!!");
else               puts("NOT VALID");
```

---

## 4. What Is the Luhn Algorithm?

The checksum is defined as:

1. Starting from the rightmost digit (check digit), move left, doubling every second digit.
2. If doubling produces a two‑digit number, add those digits (or subtract 9).
3. Sum all digits. A valid number has a sum divisible by 10.

It’s designed to catch most accidental errors (single‑digit mistakes, adjacent swaps), not to provide cryptographic security.