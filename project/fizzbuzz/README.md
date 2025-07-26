# FizzBuzz

A program that prints numbers from **1 to *n***, replacing multiples of 3 with **"Fizz"**, multiples of 5 with **"Buzz"**, and multiples of both with **"FizzBuzz"**.

---

## 1. Build & Run

```bash
cc fizzbuzz.c -o fizzbuzz
./fizzbuzz 30
```

Output (truncated):

```
1 2 Fizz 4 Buzz Fizz 7 8 Fizz Buzz 11 Fizz 13 14 FizzBuzz 16 ...
```

If you forget the argument:

```
Usage: ./fizzbuzz <number>
```

---

## 2. Code Walkthrough

### 2.1 Parse Argument

```c
if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
}
int number = atoi(argv[1]);
```

* Requires one integer on the command line.
* Uses `atoi` (simple parse, no error handling for junk like "abc").

### 2.2 Main Loop & Rules

```c
for (int i = 1; i <= number; i++) {
    if (i % 3 == 0 && i % 5 == 0)
        printf("FizzBuzz ");
    else if (i % 3 == 0)
        printf("Fizz ");
    else if (i % 5 == 0)
        printf("Buzz ");
    else
        printf("%d ", i);
}
```

Order matters: check the combined case (`i % 3 == 0 && i % 5 == 0`) **first** or you’ll never print "FizzBuzz".