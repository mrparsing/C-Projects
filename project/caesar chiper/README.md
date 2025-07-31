# Caesar Cipher (Interactive C Program)

Minimal interactive implementation of the classical Caesar shift cipher. Supports **encryption** and **decryption** of ASCII alphabetic text using a numeric key. 

> **Important:** Caesar cipher offers **no real security**. It is purely educational.

---

## 1. Quick Start

### Build

```bash
chmod +x build.sh
./build
```

### Run

```bash
./caesar_chiper
```

Follow the on-screen menu.

---

## 2. Program Flow Overview

1. Display welcome banner.
2. Loop showing a menu:

   * **1** Encrypt
   * **2** Decrypt
   * **3** Quit (any other number also exits)
3. For encrypt/decrypt:

   * Prompt for sentence.
   * Prompt for key (integer; normalized modulo 26).
   * Shift each alphabetic character accordingly.
   * Show result.
4. Return to menu until user chooses to quit.

---

## 3. Core Data & Constraints

| Item              | Value / Description                                                               |
| ----------------- | --------------------------------------------------------------------------------- |
| Buffer size       | 1024 bytes for the sentence (including null terminator)                           |
| Alphabet handled  | A–Z and a–z only (ASCII)                                                          |
| Key normalization | `key = ((key % 26) + 26) % 26` (implemented with `%` then fix if negative)        |
| Non-letters       | Left unchanged (digits, punctuation, spaces, UTF‑8 multibyte will pass byte-wise) |

---

## 4. Functions in Detail

### 4.1 `int get_key()`

**Purpose:** Prompt user, read integer key, normalize into `[0,25]`.

```c
int key;                      // user input (could be large or negative)
printf("Enter the key (number): ");
scanf("%d", &key);
key %= 26;                    // reduce magnitude
if (key < 0) key += 26;       // ensure non-negative
return key;                   // final usable key
```

*Why normalize?* The Caesar cipher conceptually shifts inside a 26-letter ring. Any integer key equivalent modulo 26 yields identical transformation.

### 4.2 `void get_sentence(char *buf, size_t size)`

**Purpose:** Read a line (possibly containing spaces) into `buf` and strip the trailing newline.

```c
fgets(buf, size, stdin);               // reads up to size-1 chars or newline
buf[strcspn(buf, "\n")] = 0;          // find newline position and replace with NUL
```

The newline removal prevents accidental extra blank line during output and ensures strings are clean for processing.

### 4.3 `char shift_char(char c, int key)`

**Purpose:** Shift a single ASCII letter by `key` positions preserving case; leave all other characters intact.

```c
if (c >= 'a' && c <= 'z')
    c = ((c - 'a' + key) % 26) + 'a';
else if (c >= 'A' && c <= 'Z')
    c = ((c - 'A' + key) % 26) + 'A';
return c;
```

*Mechanics:* Convert letter to 0–25 index (`c - base`), add key, wrap with `% 26`, add base back.

### 4.4 `void caesar_encrypt()`

**Workflow:** Gather sentence → key → shift each char forward by `key` → print.

```c
char sentence[1024];
get_sentence(sentence, sizeof sentence);
int key = get_key();
for (int i = 0; sentence[i]; ++i)
    sentence[i] = shift_char(sentence[i], key);
printf("\nEncrypted text: %s\n\n", sentence);
```

A 1-second `sleep(1)` (as in your code) is purely cosmetic (“Encrypting…” message) and can be removed to speed tests.

### 4.5 `void decrypt()`

**Approach used:** Instead of writing a dedicated reverse shifter, reuse `shift_char` with `(26 - key)`.

```c
for (int i = 0; sentence[i]; ++i)
    sentence[i] = shift_char(sentence[i], 26 - key);
```

Because addition modulo 26: `(x + key + (26 - key)) % 26 == x`.

### 4.6 `int main()`

**Highlights:**

* Prints banner.
* Menu loop using `do { ... } while (option == 1 || option == 2);` so it repeats only after encrypt/decrypt.
* After reading the numeric menu option with `scanf`, consumes leftover newline:

```c
while (getchar() != '\n');
```

* Switch executes chosen action.
* Any option other than 1 or 2 exits after printing "Bye!".

---

## 5. Example Session

```
===================================================
           Welcome to the Caesar Cipher!
  Have fun encrypting and decrypting messages. :)
===================================================

What would you like to do?
1) Encrypt
2) Decrypt
3) Quit
Enter an option: 1
Enter the sentence: attack at dawn
Enter the key (number): 5
Encrypting...

Encrypted text: fyyfhp fy ifbs

What would you like to do?
1) Encrypt
2) Decrypt
3) Quit
Enter an option: 2
Enter the sentence: fyyfhp fy ifbs
Enter the key (number): 5
Decrypting...

Decrypted text: attack at dawn

Enter an option: 3
Bye!
```