#include <gmp.h>
#include <stdio.h>
#include <stdbool.h>

// Compute GCD using Euclidean algorithm
void mcd(mpz_t result, const mpz_t a, const mpz_t b) {
    mpz_t x, y, temp;
    mpz_init_set(x, a);
    mpz_init_set(y, b);
    mpz_init(temp);

    while (mpz_cmp_ui(y, 0) != 0) {
        mpz_set(temp, y);
        mpz_mod(y, x, y);
        mpz_set(x, temp);
    }

    mpz_set(result, x);

    mpz_clear(x);
    mpz_clear(y);
    mpz_clear(temp);
}

// Compute modular inverse of e mod m using extended Euclidean algorithm
bool inverso_modulare(mpz_t result, const mpz_t e, const mpz_t m) {
    mpz_t r, new_r, t, new_t, q, temp;
    mpz_inits(r, new_r, t, new_t, q, temp, NULL);

    mpz_set(r, m);
    mpz_set(new_r, e);
    mpz_set_ui(t, 0);
    mpz_set_ui(new_t, 1);

    while (mpz_cmp_ui(new_r, 0) != 0) {
        mpz_fdiv_q(q, r, new_r);

        mpz_mul(temp, q, new_t);
        mpz_sub(temp, t, temp);
        mpz_set(t, new_t);
        mpz_set(new_t, temp);

        mpz_mul(temp, q, new_r);
        mpz_sub(temp, r, temp);
        mpz_set(r, new_r);
        mpz_set(new_r, temp);
    }

    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_clears(r, new_r, t, new_t, q, temp, NULL);
        return false;
    }

    if (mpz_sgn(t) < 0) {
        mpz_add(t, t, m);
    }

    mpz_set(result, t);

    mpz_clears(r, new_r, t, new_t, q, temp, NULL);
    return true;
}

// RSA encryption: C = M^e mod n
void rsa_encrypt(mpz_t ciphertext, const mpz_t message, const mpz_t e, const mpz_t n) {
    mpz_powm(ciphertext, message, e, n);
}

// RSA decryption: M = C^d mod n
void rsa_decrypt(mpz_t message, const mpz_t ciphertext, const mpz_t d, const mpz_t n) {
    mpz_powm(message, ciphertext, d, n);
}

int main() {
    mpz_t p, q, n, phi_n, p_minus_1, q_minus_1;
    mpz_inits(p, q, n, phi_n, p_minus_1, q_minus_1, NULL);

    gmp_randstate_t stato;
    gmp_randinit_mt(stato);
    gmp_randseed_ui(stato, 1234);

    mpz_urandomb(p, stato, 64);
    mpz_urandomb(q, stato, 64);
    mpz_nextprime(p, p);
    mpz_nextprime(q, q);

    gmp_printf("Generated primes:\np = %Zd\nq = %Zd\n", p, q);

    mpz_mul(n, p, q);

    mpz_sub_ui(p_minus_1, p, 1);
    mpz_sub_ui(q_minus_1, q, 1);
    mpz_mul(phi_n, p_minus_1, q_minus_1);

    gmp_printf("Modulus n = %Zd\n", n);
    gmp_printf("Euler's totient phi(n) = %Zd\n", phi_n);

    mpz_t e;
    mpz_init_set_ui(e, 65537);

    mpz_t gcd;
    mpz_init(gcd);
    mpz_gcd(gcd, e, phi_n);

    if (mpz_cmp_ui(gcd, 1) != 0) {
        printf("ERROR: 65537 is not coprime with phi(n)\n");
        return 1;
    }

    mpz_t d;
    mpz_init(d);

    if (!inverso_modulare(d, e, phi_n)) {
        printf("ERROR: Could not compute modular inverse\n");
        return 1;
    }

    gmp_printf("Public key (e, n): (%Zd, %Zd)\n", e, n);
    gmp_printf("Private key (d, n): (%Zd, %Zd)\n", d, n);

    // User input
    char input_str[1024];
    printf("Enter a message (as a number < n): ");
    scanf("%1023s", input_str);

    mpz_t message, ciphertext, decrypted;
    mpz_inits(message, ciphertext, decrypted, NULL);
    mpz_set_str(message, input_str, 10);

    if (mpz_cmp(message, n) >= 0) {
        printf("ERROR: message must be smaller than n\n");
        return 1;
    }

    rsa_encrypt(ciphertext, message, e, n);
    gmp_printf("Encrypted ciphertext: %Zd\n", ciphertext);

    rsa_decrypt(decrypted, ciphertext, d, n);
    gmp_printf("Decrypted message: %Zd\n", decrypted);

    // Clear memory
    mpz_clears(p, q, n, phi_n, p_minus_1, q_minus_1, e, d, gcd, message, ciphertext, decrypted, NULL);
    gmp_randclear(stato);

    return 0;
}