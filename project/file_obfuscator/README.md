# Byte Transformer (NOT / XOR)

A very small C utility that applies a **bitwise NOT** or a **byte-wise XOR** to the contents of a file and writes the transformed bytes to an output file. Useful as a didactic example for basic bitwise operations, simple (weak) obfuscation, or pipeline experimentation.

> **Security Note:** This is **not encryption**. Both NOT and XOR with a single-byte key are trivially reversible and provide zero real security. Use a proper cryptographic library for any serious purpose.

---

## 1. Features

* Two modes:

  * `not` – invert every bit (`0xAB -> 0x54`). Equivalent to `~byte & 0xFF` to keep it in 0–255.
  * `xor <key>` – XOR every byte with a user‑provided integer key (truncated to 8 bits).
* Works on *any* file type (binary or text) because it treats data as raw bytes.
* Simple, dependency‑free ANSI C (except for the C standard library headers).

---

## 2. Build

```bash
chmod +x build.sh
./build
```

## 3. Usage

```
Usage:
  ./file_obfuscator not  input.txt output.bin
  ./file_obfuscator xor <key> input.bin output.bin
```

Parameters:

| Arg      | Meaning                  | Notes                                    |
| -------- | ------------------------ | ---------------------------------------- |
| `mode`   | `not` or `xor`           | selects transformation                   |
| `key`    | integer (only for `xor`) | Only lowest 8 bits matter (`key & 0xFF`) |
| `input`  | source file              | opened in binary (`rb`)                  |
| `output` | destination file         | written in binary (`wb`)                 |

### Examples

Invert a text file:

```bash
./file_obfuscator not message.txt message.inv
```

Restore the original (apply NOT again):

```bash
./file_obfuscator not message.inv message_restored.txt
```

XOR a PNG with key 42:

```bash
./file_obfuscator xor 42 image.png image.xor
```

Revert:

```bash
./file_obfuscator xor 42 image.xor image_restored.png
```

---

## 4. How It Works (Step by Step)

### 4.1 `main` argument parsing

```c
if (argc < 4) {
    fprintf(stderr, "Usage...\n");
    return 1;
}
```

* Requires at least 3 extra args. Two valid patterns:

  1. `not input output` (argc == 4)
  2. `xor key input output` (argc == 5)

Logic branches:

```c
if (strcmp(mode, "not") == 0 && argc == 4) { /* ... */ }
else if (strcmp(mode, "xor") == 0 && argc == 5) { /* ... */ }
else { /* invalid */ }
```

Ensures we don't accidentally read a missing key or mis-assign filenames.

### 4.2 Opening files

```c
FILE *in = fopen(input_file, "rb");
FILE *out = fopen(output_file, "wb");
```

Binary flags (`rb`/`wb`) avoid unintended newline translations on Windows and make it safe for arbitrary binary data.

### 4.3 Processing loop

```c
int c;
while ((c = fgetc(in)) != EOF) {
    fputc(transform(c, mode, key), out);
}
```

Reads one byte at a time as an `int` (needed to detect EOF). Each byte goes through `transform()` and is written immediately. Memory footprint stays O(1) w\.r.t. file size.

### 4.4 The `transform` function

```c
int transform(int c, const char *mode, int key) {
    if (strcmp(mode, "not") == 0) {
        return ~c & 0xFF;
    } else if (strcmp(mode, "xor") == 0) {
        return c ^ key;
    } else {
        return c;
    }
}
```

* **NOT mode:** bitwise negation flips all bits. We mask with `0xFF` to ensure the result remains a valid 8‑bit value since `~c` operates on an `int` which might have more than 8 bits.
* **XOR mode:** `c ^ key` does the classic XOR. Since `c` is in `0..255`, only the lowest 8 bits of `key` matter. Example: if `key = 300`, then `300 & 0xFF = 44` (0x2C). If you want strict control you could explicitly use `key & 0xFF`.
* **Fallback:** returns the original byte if an unsupported mode somehow sneaks through.

### 4.5 Cleanup

Files are closed after the loop. No dynamic memory is allocated in this program; all state is automatic variables.

---

## 5. Reversibility

| Mode | Apply twice?      | Reason                         |
| ---- | ----------------- | ------------------------------ |
| NOT  | Restores original | `~~x == x` for any bit pattern |
| XOR  | Restores original | `(x ^ k) ^ k == x`             |

To undo a transformation, just run the identical command again on the transformed file.

---

## 6. Possible Extensions

* **Multi-byte XOR key**: cycle through a user-provided hex string.
* **Streaming stdin/stdout**: allow `-` for input/output to use pipes.
* **Hex dump mode**: print before/after for debugging.
* **Progress indicator** for large files (use `fstat` for size).

---

## 7. Security Reminder

Reiterating: this tool **only obfuscates**. Anyone can recover original data instantly if they have the output. DO NOT use it for passwords, personal data, or anything requiring confidentiality.