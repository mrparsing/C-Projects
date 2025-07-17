#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Initialize the RC4 state array S using the key scheduling algorithm (KSA)
void init_rc4(uint8_t *S, const uint8_t *key, size_t keylen)
{
    // Initialize the permutation array with values from 0 to 255
    for (int i = 0; i < 256; i++)
    {
        S[i] = i;
    }

    int i = 0;
    int j = 0;

    // Perform the key scheduling
    for (int i = 0; i < 256; i++)
    {
        j = (j + S[i] + key[i % keylen]) % 256;

        // Swap S[i] and S[j]
        uint8_t tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;
    }
}

// Encrypt or decrypt data using RC4 (encryption and decryption are symmetric)
void rc4_crypt(uint8_t *S, const uint8_t *input, uint8_t *output, size_t len)
{
    int i = 0;
    int j = 0;

    for (size_t n = 0; n < len; n++)
    {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;

        // Swap S[i] and S[j]
        uint8_t tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;

        // Generate pseudo-random byte and XOR it with the input
        uint8_t t = (S[i] + S[j]) % 256;
        uint8_t k = S[t];

        output[n] = input[n] ^ k;
    }
}

int main()
{
    char key[256];      // Buffer to store the key input from the user
    char msg[1024];     // Buffer to store the message input from the user

    // Get key from user input
    printf("Insert the key: ");
    fgets(key, sizeof(key), stdin);
    key[strcspn(key, "\n")] = '\0'; // Remove trailing newline

    // Get message from user input
    printf("Insert the message: ");
    fgets(msg, sizeof(msg), stdin);
    msg[strcspn(msg, "\n")] = '\0';

    size_t msglen = strlen(msg);  // Get length of the message

    // Buffers to hold the ciphertext and decrypted message
    uint8_t ciphertext[msglen];
    uint8_t decrypted[msglen];

    // Encrypt the message
    uint8_t SBOX[256];
    memset(SBOX, 0, sizeof(SBOX)); // Optional: clear memory before use
    init_rc4(SBOX, (uint8_t *)key, strlen(key));
    rc4_crypt(SBOX, (uint8_t *)msg, ciphertext, msglen);

    // Print the encrypted message in hexadecimal format
    printf("Encrypted message (HEX): ");
    for (size_t i = 0; i < msglen; i++)
    {
        printf("%02X", ciphertext[i]);
    }

    // Decrypt the message
    uint8_t SBOX2[256];
    memset(SBOX2, 0, sizeof(SBOX2));
    init_rc4(SBOX2, (uint8_t *)key, strlen(key));
    rc4_crypt(SBOX2, ciphertext, decrypted, msglen);

    // Print the decrypted message
    printf("\nDecrypted message: ");
    printf("%s\n", decrypted);

    // Clear sensitive data from memory (optional but good practice)
    memset(key, 0, sizeof(key));
    memset(msg, 0, sizeof(msg));
    memset(ciphertext, 0, sizeof(ciphertext));
    memset(decrypted, 0, sizeof(decrypted));
}