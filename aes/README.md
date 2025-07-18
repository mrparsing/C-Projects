# AES-128

A **minimal, educational implementation** of core AES-128 encryption & decryption steps in C:

* Key size: 128 bits (16 bytes)
* Block size: 128 bits (16 bytes)
* 10 rounds (9 full + final round without MixColumns)

The program performs **one-block ECB-like** encrypt/decrypt of the ASCII string `"asdfghjklzxcvbnm"` using the same bytes as key. It then prints the raw transformed bytes as characters (not hex), which may yield unreadable output for general inputs.

> ⚠️ **Not production‑ready:** Missing padding, block mode (CBC/GCM), input validation, constant-time precautions, and multiple security hardenings. Use ONLY for learning.

---

## 1. AES State Representation

AES conceptually operates on a 4×4 byte matrix called the *state*.

```
index k (0..15)  -> mapping used in your code:
for (j = 0; j < 4; ++j)      // column-major fill
  for (i = 0; i < 4; ++i)
      state[i][j] = input[k++];
```

So bytes are loaded **column-major**: `state[row][col]`. This matches the AES spec layout: first 4 bytes form column 0, next 4 bytes column 1, etc.

---

## 2. High-Level Encryption Flow

1. **KeyExpansion** → expands 16-byte key into 176 bytes (11 round keys × 16).
2. **Initial AddRoundKey** (XOR state with round key 0).
3. **Rounds 1–9** (the “full” rounds):

   * SubBytes
   * ShiftRows
   * MixColumns
   * AddRoundKey (round i)
4. **Final Round (10th)**:

   * SubBytes
   * ShiftRows
   * AddRoundKey (round 10)

Decryption reverses with inverse transformations and reverse key order.

---

## 3. Core Transformation Functions

### 3.1 `subBytes`

```c
void subBytes(uint8_t state[4][4], uint8_t *sbox) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] = sbox[state[i][j]]; // nonlinear byte substitution
}
```

Each byte is passed through a fixed 256-entry S‑box (provides nonlinearity & confusion). Your S‑box array matches the standard Rijndael S-box.

### 3.2 `shiftRows`

```c
void shiftRows(uint8_t state[4][4]) {
    rotateLeft(state[1], 1); // 2nd row left 1
    rotateLeft(state[2], 2); // 3rd row left 2
    rotateLeft(state[3], 3); // 4th row left 3
}
```

Row 0 unchanged. Cyclic left shifts give inter-column diffusion. Helper:

```c
void rotateLeft(uint8_t *row, int n) {
    for (int r = 0; r < n; r++) {
        uint8_t tmp = row[0];
        row[0]=row[1]; row[1]=row[2]; row[2]=row[3]; row[3]=tmp;
    }
}
```

No loop unrolling or optimization; fine for clarity.

### 3.3 `mixColumns`

Operates on each column as a 4-term polynomial over GF(2^8): multiply by fixed matrix:

```
[02 03 01 01]
[01 02 03 01]
[01 01 02 03]
[03 01 01 02]
```

Implementation:

```c
state[0][j] = gmult(0x02,s0) ^ gmult(0x03,s1) ^ s2 ^ s3;
state[1][j] = s0 ^ gmult(0x02,s1) ^ gmult(0x03,s2) ^ s3;
state[2][j] = s0 ^ s1 ^ gmult(0x02,s2) ^ gmult(0x03,s3);
state[3][j] = gmult(0x03,s0) ^ s1 ^ s2 ^ gmult(0x02,s3);
```

`gmult` performs GF(2^8) multiply with the AES irreducible polynomial x^8+x^4+x^3+x+1 (represented by conditional XOR with 0x1b when the high bit shifts out):

```c
uint8_t gmult(uint8_t a, uint8_t b){
    uint8_t p=0;
    for(int i=0;i<8;i++){
        if(b & 1) p ^= a;          // add a if lowest bit of b set
        uint8_t hi = a & 0x80;     // remember if overflow
        a <<= 1;                   // multiply a by x
        if(hi) a ^= 0x1b;          // modular reduction
        b >>= 1;                   // next bit
    }
    return p;
}
```

### 3.4 `addRoundKey`

XOR state bytes with 16-byte round key segment:

```c
void addRoundKey(uint8_t s[4][4], uint8_t *rk){
    int k=0; for(int j=0;j<4;j++) for(int i=0;i<4;i++) s[i][j] ^= rk[k++];
}
```

Uses column-major order consistent with state layout and key schedule output.

---

## 4. Key Expansion (`keyExpansion`)

Expands original 16-byte cipher key to 176 bytes (11 round keys). Your logic:

1. Copy initial 16 key bytes.
2. While generated < 176:

   * Copy previous 4-byte word into `t`.
   * If at a 16-byte boundary (i.e. new word index multiple of 4):

     * RotWord (1-byte left rotate)
     * SubWord (S-box each byte)
     * XOR first byte with `rcon[rconIteration++]`.
   * XOR `t` with the word 4 positions back (i.e. `i-4`) to form new word.

Snippet:

```c
if(bytesGenerated % 16 == 0){
    // RotWord
    uint8_t tmp = t[0]; t[0]=t[1]; t[1]=t[2]; t[2]=t[3]; t[3]=tmp;
    // SubWord
    for(int k=0;k<4;k++) t[k] = sbox[t[k]];
    t[0] ^= rcon[rconIteration++];
}
expandedKeys[bytesGenerated] = expandedKeys[bytesGenerated-16] ^ t[k];
```

Output layout: round i key starts at `expandedKeys + 16*i`.

> **Observation:** Code assumes AES-128 only (Nk=4, Nr=10). For AES-192/256 more conditions apply (extra SubWord on certain words).

---

## 5. Decryption Primitives

Inverse sequence uses:

* `invSubBytes` with `inv_sbox` (standard inverse S-box).
* `invShiftRows` (right rotations matching left shifts).
* `invMixColumns` multiplies columns by inverse matrix:

```
[0e 0b 0d 09]
[09 0e 0b 0d]
[0d 09 0e 0b]
[0b 0d 09 0e]
```

Implementation mirrors forward MixColumns with different constants.

Decryption order per round (except final) is:

```
InvShiftRows -> InvSubBytes -> AddRoundKey -> InvMixColumns
```

Final round omits InvMixColumns, symmetrically to encryption omitting MixColumns.

---

## 6. Main Function

```c
uint8_t input[16] = "asdfghjklzxcvbnm"; // 16 ASCII bytes
uint8_t key[16]   = "asdfghjklzxcvbnm"; // identical key
keyExpansion(key, expandedKeys, sbox);

// Load state column-major
k=0; for(j=0;j<4;j++) for(i=0;i<4;i++) state[i][j] = input[k++];

// Initial round key
addRoundKey(state, &expandedKeys[0]);

// Rounds 1..9
for(round=1; round<=9; ++round){
    subBytes(state,sbox);
    shiftRows(state);
    mixColumns(state);
    addRoundKey(state, &expandedKeys[round*16]);
}

// Final round (round 10)
subBytes(state,sbox);
shiftRows(state);
addRoundKey(state, &expandedKeys[160]); // 10th key
```

Then ciphertext is printed *as characters*. Many ciphertext bytes may not be printable ASCII — using hex is clearer:

```c
printf("%02x", state[i][j]);
```

Decryption then:

1. **Mistake / Non-standard Order:** It does `addRoundKey(state, &expandedKeys[160]);` *immediately again* before starting inverse rounds. Standard AES decryption would start with that AddRoundKey only once (the same key already applied at end of encryption). Because encryption already applied final AddRoundKey, you should NOT re-apply it before inverse rounds; instead you start with it logically when decrypting a fresh ciphertext. In this code, since encryption and decryption happen back-to-back in the same state variable, redoing the final AddRoundKey cancels itself (XOR twice with same key → original pre-final-round state), effectively *undoing* the last key addition first. This still yields correct plaintext in this in-memory sequence, but diverges from typical presentation. Educational point.
2. Inverse rounds 9→1: `InvShiftRows -> InvSubBytes -> AddRoundKey -> InvMixColumns`.
3. Final inverse round: `InvShiftRows -> InvSubBytes -> AddRoundKey (round 0)`.

Plaintext printed as characters, returning to the original string.