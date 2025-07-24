#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Ask the user for a key (integer).                                           
 * The key is reduced modulo 26 so that large values wrap within the alphabet. 
 * Negative keys are normalized into the 0..25 range.                          */
int get_key()
{
    int key;

    printf("Enter the key (number): ");
    scanf("%d", &key);

    key %= 26;
    if (key < 0)
    {
        key += 26;
    }

    return key;
}

/* Read a line of text from stdin into the provided buffer.                    
 * Trailing newline (if present) is stripped.                                  */
void get_sentence(char *buf, size_t size)
{
    printf("Enter the sentence: ");
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0;
}

/* Shift a single character by 'key' positions within the alphabet.           
 * Only ASCII letters [A-Z] and [a-z] are shifted; all others are returned      
 * unchanged.                                                                  */
char shift_char(char c, int key)
{
    if (c >= 'a' && c <= 'z')
    {
        c = ((c - 'a' + key) % 26) + 'a';
    }
    else if (c >= 'A' && c <= 'Z')
    {
        c = ((c - 'A' + key) % 26) + 'A';
    }

    return c;
}

/* Encrypt flow:                                                               
 *   1. Prompt for the sentence.                                               
 *   2. Prompt for the key.                                                    
 *   3. Apply shift_char() to each character using the normalized key.         
 *   4. Print the encrypted result.                                            */
void caesar_encrypt()
{
    char sentence[1024];

    get_sentence(sentence, sizeof(sentence));

    int key = get_key();

    printf("Encrypting...\n");
    sleep(1);

    for (int i = 0; sentence[i] != '\0'; i++)
    {
        sentence[i] = shift_char(sentence[i], key);
    }

    printf("\nEncrypted text: %s\n\n", sentence);
}

/* Decrypt flow mirrors encrypt:                                               
 *   1. Prompt for the sentence.                                               
 *   2. Prompt for the key.                                                    
 *   3. To decrypt, shift each character by (26 - key) (original logic).       
 *   4. Print the decrypted result.                                            */
void decrypt()
{
    char sentence[1024];

    get_sentence(sentence, sizeof(sentence));

    int key = get_key();

    printf("Decrypting...\n");
    sleep(1);

    for (int i = 0; sentence[i] != '\0'; i++)
    {
        sentence[i] = shift_char(sentence[i], 26 - key);
    }

    printf("\nDecrypted text: %s\n\n", sentence);
}

int main()
{
    int option;

    /* Friendly welcome banner */
    printf("===================================================\n");
    printf("           Welcome to the Caesar Cipher!\n");
    printf("  Have fun encrypting and decrypting messages. :)\n");
    printf("===================================================\n\n");

    do
    {
        printf("What would you like to do?\n");
        printf("1) Encrypt\n");
        printf("2) Decrypt\n");
        printf("3) Quit\n");
        printf("Enter an option: ");
        scanf("%d", &option);
        while (getchar() != '\n')
            ; // flush leftover input (newline, extra chars)

        switch (option)
        {
        case 1:
            caesar_encrypt();
            break;
        case 2:
            decrypt();
            break;
        default:
            printf("Bye!\n");
            break;
        }
    } while (option == 1 || option == 2);

    return 0;
}