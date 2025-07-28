# Number Base Converter

A command-line tool written in C to convert numbers between Decimal, Binary, and Hexadecimal systems.

---

## Features

* Converts numbers from a source base to a target base.
* Supported bases: **D**ecimal (base 10), **B**inary (base 2), and **H**exadecimal (base 16).
* Correctly handles hexadecimal digits `A-F` (case-insensitive).
* Includes input validation for bases and numbers.

---

## ⚙️ How to Compile

You need a C compiler like **GCC**. Save the code as `converter.c` and run the following command in your terminal:

```bash
gcc converter.c -o converter
```

## How to Use
The program is run from the terminal with three required arguments:

```bash
./converter <number> <source_base> <target_base>
<number>: The number you want to convert.
```
```
<source_base>: The base of the input number. Use D, B, or H.
```
```
<target_base>: The base you want to convert to. Use D, B, or H.
```

## How It Works
The program's logic is efficient and relies on standard C library functions for robust parsing and conversion.

1. Input Parsing

    The program first reads the three command-line arguments. The key function used for parsing the input number is strtol (String to Long). Unlike atoi, strtol can parse a string representation of a number from a specified base (2 for binary, 10 for decimal, or 16 for hexadecimal).

    ```C
    // 'base' is set to 2, 10, or 16 depending on user input
    long number = strtol(input_str, &end, base);
    ```
    This single line correctly handles input from any of the supported bases, including hexadecimal numbers with letters (A-F). The program also checks if strtol was able to parse the entire string, providing an error if the number is invalid for its specified base (e.g., 123 for binary).

2. Output Generation

    Once the input number is stored in a standard long integer variable, generating the output is straightforward:

    For Decimal and Hexadecimal: The program uses the standard printf format specifiers %ld (long decimal) and %lX (long hexadecimal) to print the result directly.

    For Binary: A dedicated function, print_as_binary, is used. This function iterates through each bit of the integer, from the most significant bit (MSB) to the least significant bit (LSB). It uses bitwise operations to check the value of each bit (0 or 1) and prints it.

    ```c
    // (n >> i) shifts the i-th bit to the rightmost position.
    // & 1 isolates that bit.
    int bit = (n >> i) & 1;
    ```

    This approach ensures that the binary representation is always printed in the correct order and avoids common conversion bugs.

