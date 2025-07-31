# Minimal MD5 Implementation (C)

A compact, didactic MD5 hash implementation in ANSI C.

> **Disclaimer:** MD5 is **cryptographically broken** (collisions are practical). Use this only for learning or non‑security checksums. For real security pick SHA‑256, BLAKE2, SHA‑3, etc.

---

## 1. Requirements

Any C toolchain (GCC / Clang / MSVC / TinyCC). No external libraries required.

### Compile Examples

**Linux / macOS:**

```bash
chmod +x build.sh
./build
./md5
```

---

## 2. Usage

Run, type a line, press Enter; program prints the 128‑bit digest as 32 lowercase hex characters.

```
Enter a message: hello
5d41402abc4b2a76b9719d911017c592
```

---

## 3. High‑Level Algorithm Recap

MD5 processes the message in **512‑bit (64‑byte) blocks** after padding. It maintains a 128‑bit internal state (A,B,C,D). For each block it performs 64 operations grouped into 4 rounds using non‑linear functions (F,G,H,I), constants `K[i]` derived from `abs(sin(i+1))`, and per‑step left rotations `s[i]`. After the final block the state words (little‑endian) form the digest.

Pipeline:

1. **Pad** message: append `0x80`, then `0x00` bytes until length ≡ 56 (mod 64); append original bit length as 64‑bit little‑endian.
2. **Initialize** state: `A=0x67452301`, `B=0xefcdab89`, `C=0x98badcfe`, `D=0x10325476`.
3. **For each 512‑bit chunk**: interpret as 16 little‑endian 32‑bit words; perform 64 steps.
4. **Add** working variables back into the state (mod 2^32).
5. **Output** concatenated (A || B || C || D) little‑endian.

---

## 4. Source File Tour

### 4.1 Rotation Amounts & Constants

```c
static const uint32_t s[] = { /* 64 per-step left-rotate counts */ };
static const uint32_t K[] = { /* floor(2^32 * fabs(sin(i+1))) */ };
```

* `s[i]` supplies how many bits to rotate in step `i` (data‑dependent diffusion).
* `K[i]` are fixed additive constants preventing simple symmetries.

### 4.2 Left Rotation Macro

```c
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))
```

Wraps the standard circular left shift; assumes `c` in \[0,31]. Compiler inlines this.

### 4.3 Padding Function

```c
uint8_t *md5_pad(const uint8_t *msg, size_t len, size_t *new_len)
```

**Goal:** produce a new buffer whose length is a multiple of 64, with 8 trailing bytes encoding the original length *in bits*.

Key steps:

1. Start from original `len` and add 1 byte (`0x80`).
2. Increase `padded_len` until `(padded_len % 64) == 56` (i.e. 448 bits).
3. Add 8 bytes => total length divisible by 64.
4. Copy original message, set the first padding byte, leave zero bytes (calloc).
5. Write `uint64_t bit_len = len * 8` at the end (little‑endian) per MD5 spec.

**Note:** Function returns allocated heap memory (caller frees).

### 4.4 Core Hash Function

```c
void md5(const uint8_t *initial_msg, size_t len, uint8_t digest[16])
```

Workflow inside:

1. Call `md5_pad` → `msg`, `new_len`.
2. Initialize state registers A,B,C,D with IV constants.
3. For each 64‑byte chunk:

   * Interpret as `uint32_t *w` (16 words) — relies on **little‑endian host** or at least consistent with MD5 spec. (On big‑endian you’d need byte swaps.)
   * Copy state into `a,b,c,d` (working variables).
   * Loop `i` from 0..63:

     * Compute round function & message index:

       * Round 1 (0–15): `F = (b & c) | (~b & d)`, `g = i`.
       * Round 2 (16–31): `F = (d & b) | (~d & c)`, `g = (5*i+1) % 16`.
       * Round 3 (32–47): `F = b ^ c ^ d`, `g = (3*i+5) % 16`.
       * Round 4 (48–63): `F = c ^ (b | ~d)`, `g = (7*i) % 16`.
     * Accumulate:

       ```c
       uint32_t temp = d;      // rotate registers
       d = c;
       c = b;
       b = b + LEFTROTATE(a + f + K[i] + w[g], s[i]);
       a = temp;
       ```
   * Add working vars back: `A += a; B += b; C += c; D += d;` (mod 2^32 via uint overflow).
4. Free padded buffer.
5. Emit digest: copy each 32‑bit state word in little‑endian order into `digest[16]`.

### 4.5 Test Driver

```c
int main(void) { ... }
```

* Reads a line (up to 1023 chars) using `fgets`.
* Strips trailing `\n`.
* Calls `md5` and prints 16 bytes as 32 hex characters with `%02x` loop.

---

## 8. Core Loop

```c
for (int i = 0; i < 64; ++i) {
    uint32_t f, g;
    if (i < 16) { f = (b & c) | (~b & d);         g = i; }           // Round 1
    else if (i < 32) { f = (d & b) | (~d & c);    g = (5*i + 1) % 16; } // Round 2
    else if (i < 48) { f = b ^ c ^ d;             g = (3*i + 5) % 16; } // Round 3
    else { f = c ^ (b | ~d);                      g = (7*i) % 16; }    // Round 4

    uint32_t temp = d;    // Save d before shifting the register window
    d = c;                // Each step shifts (a,b,c,d) one position
    c = b;
    b = b + LEFTROTATE(a + f + K[i] + w[g], s[i]); // Non-linear mix + constant + data word + rotate
    a = temp;             // Old d becomes new a
}
```

**Why this order?** It matches the MD5 spec’s register rotation pattern; each operation uses all four registers and injects one message word, then rotates the roles.