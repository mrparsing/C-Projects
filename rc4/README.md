# 🔑 RC4 Stream Cipher in C

This is a simple implementation of the RC4 encryption algorithm written in C. It allows the user to input a key and a message, encrypts the message using RC4, and then decrypts it to verify correctness.

⚠️ Note: RC4 is considered cryptographically broken and should not be used in real-world applications. This program is for educational purposes only.

---

# What This Program Does

✅ Steps:
1.	Asks the user for:
- A key (string)
- A message (string)
2.	Initializes the RC4 internal state array (S) using the Key Scheduling Algorithm (KSA).
3.	Uses the Pseudo-Random Generation Algorithm (PRGA) to:
- Encrypt the message
- Print the encrypted output in hex format
- Decrypt the ciphertext back into plaintext
- Print the decrypted message

# How It Works

🔐 RC4 Overview

RC4 is a symmetric stream cipher, which means:
- The same function is used for both encryption and decryption.
- It generates a pseudo-random byte stream that is XOR-ed with the plaintext or ciphertext.

## 🔧 Key Scheduling Algorithm (KSA)
```c
void init_rc4(uint8_t *S, const uint8_t *key, size_t keylen);
```
- Initializes the S array with values from 0 to 255.
- Scrambles S using the provided key to make it unpredictable.

## ⚙️ Pseudo-Random Generation Algorithm (PRGA)
```c
void rc4_crypt(uint8_t *S, const uint8_t *input, uint8_t *output, size_t len);
```
- Generates a pseudo-random stream based on S.
- XORs each byte of the stream with the input message to produce the ciphertext or decrypted message.


Example:
```
Insert the key: 42
Insert the message: hello
Encrypted message: ���j
FA03EEF66A
Encrypted message: hello
```

# 📌 Notes
- This implementation uses uint8_t (from <stdint.h>) to handle byte-level operations.
- The encryption and decryption both use the same function thanks to the symmetric nature of RC4.
- You must reinitialize the state array (S) before decryption, or the result will be wrong.

---

# 📚 References
- RC4 on Wikipedia: https://en.wikipedia.org/wiki/RC4
- Understanding KSA & PRGA: https://crypto.stackexchange.com/questions/2516