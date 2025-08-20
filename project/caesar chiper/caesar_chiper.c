#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

void get_sentence(char *buf, size_t size)
{
    printf("Enter the sentence: ");
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0;
}

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
            ;

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