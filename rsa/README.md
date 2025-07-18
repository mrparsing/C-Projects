# 🔐 RSA Encryption and Decryption in C with GMP

This project demonstrates how RSA works under the hood, using C and the GMP library for handling big numbers.

The program:
- Generates two large random primes p and q
- Computes the RSA modulus n = p * q
- Calculates Euler’s totient φ(n) = (p - 1)(q - 1)
- Chooses a public exponent e = 65537 (a common safe choice)
- Computes the private key d such that e * d ≡ 1 mod φ(n)
- Asks the user for a number M as the message
- Encrypts it: C = M^e mod n
- Decrypts it: M = C^d mod n

⚠️ This is a didactic implementation. It is not secure for real-world use because it uses very small key sizes (64 bits).

---

🛠 Requirements

Install the GMP development libraries.

Debian/Ubuntu:
```
sudo apt install libgmp-dev
```
macOS (Homebrew):
```
brew install gmp
```

---

🧱 What the Program Does – Step by Step

1. Generate Random Primes p and q
```c
mpz_urandomb(p, stato, 64);
mpz_nextprime(p, p);
```

It picks two random 64-bit numbers and rounds them up to the next prime using mpz_nextprime.

2. Compute RSA Parameters
- n = p * q is the modulus used in both the public and private key.
- phi(n) = (p-1)*(q-1) is Euler’s totient function.

These values are essential for RSA key generation.

3. Choose the Public Exponent e
```c
mpz_set_ui(e, 65537);
```

65537 is a very common public exponent in RSA. It’s prime, not too small, and efficient for computation.

4. Compute the Private Exponent d

Using the extended Euclidean algorithm, the program computes:
```
d ≡ e⁻¹ mod φ(n)
```

i.e., d is the modular inverse of e.

5. Ask the User for a Message

The user inputs a number (as a string) representing the message M, which must be:
```
0 < M < n
```

This simulates sending an encrypted message.

6. Encrypt the Message
```
C = M^e mod n
```

This uses mpz_powm for fast modular exponentiation.

7. Decrypt the Message
```
M = C^d mod n
```

This verifies the encryption process was correct.

---

## Sample Output

Generated primes:
```
Generated primes:
p = 15546163094340153703
q = 14092299221327315917
Modulus n = 219081182068997204166862670282218390651
Euler's totient phi(n) = 219081182068997204137224207966550921032
Public key (e, n): (65537, 219081182068997204166862670282218390651)
Private key (d, n): (171465448095962183069268372052860161321, 219081182068997204166862670282218390651)
Enter a message (as a number < n): 42
Encrypted ciphertext: 58301757177206975099894101113386131593
Decrypted message: 42
```


## 🔒 Security Notice

This is not secure RSA:
- 64-bit primes are way too small.
- There’s no padding (e.g. OAEP), which is essential in real crypto.

Use libraries like OpenSSL for anything serious.