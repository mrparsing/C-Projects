# Vigenère Cipher

A straightforward command‑line tool that encrypts a message with a **Vigenère‑like cipher**. It:

* Accepts a plaintext message and a key from stdin.
* Shifts each alphabetic character by the corresponding key letter (A → 0, B → 1, … Z → 25).
* Preserves original case; non‑letters pass through unchanged.
* Loops the key when it’s shorter than the message.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build.sh
./vigenere
Enter the message: Attack at dawn!
Enter the key:  LEMON
Encrypting...
Lxfopv ef rnhr!
```

---

## 2. Code Walkthrough

### 2.1 Input Helpers

```c
void get_sentence(char *msg, char *buf, size_t size) {
    printf("%s ", msg);
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0; // strip newline
}
```

* Reused for both message and key prompts.
* Strips trailing `\n` from `fgets` via `strcspn`.

### 2.2 Character Shift

```c
char shift_char(char c, int key) {
    if (c >= 'a' && c <= 'z')
        return ((c-'a'+key)%26)+'a';
    if (c >= 'A' && c <= 'Z')
        return ((c-'A'+key)%26)+'A';
    return c; // punctuation/spaces unchanged
}
```

Case‑aware, modulus 26.

### 2.3 Key Letter → Shift Value

```c
int get_idx_from_char(char c) {
    c = toupper(c);
    if (c >= 'A' && c <= 'Z') return c - 'A';
    return -1; // non‑alphabet key char
}
```

`get_idx_from_char('C')` → 2.

### 2.4 Encryption Loop

```c
int j = 0; // key index
for (int i = 0; i < strlen(sentence); ++i) {
    char c = sentence[i];
    if (isalpha(c)) {
        int idx = get_idx_from_char(key[j % strlen(key)]);
        putchar(shift_char(c, idx));
        ++j;
    } else putchar(c);
}
```

* Advances key index **only when the plaintext char is alphabetic** (classic Vigenère behaviour).
* Key reused cyclically via modulo.

---

## 3. Vigenère Cipher Background

The classical Vigenère cipher adds a repeating key to the plaintext:

$$
C_i = (P_i + K_{i \bmod m}) \bmod 26
$$

where *P* is plaintext index (0‑25), *K* is key index, and *C* is ciphertext.

Although stronger than a Caesar shift, it’s **not** secure against frequency analysis (Kasiski test, Friedman test) when the key is short.