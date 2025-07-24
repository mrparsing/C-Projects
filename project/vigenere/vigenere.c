#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// Prompt the user and get a string input
void get_sentence(char *msg, char *buf, size_t size)
{
    printf("%s ", msg);
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0; // Remove trailing newline character
}

// Shift a character by a given key (modulo 26), preserving case
char shift_char(char c, int key)
{
    if (c >= 'a' && c <= 'z')
    {
        return ((c - 'a' + key) % 26) + 'a';
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return ((c - 'A' + key) % 26) + 'A';
    }
    return c; // Non-alphabetical characters stay unchanged
}

// Get index from character ('A' or 'a' -> 0, 'B' -> 1, ..., 'Z' -> 25)
int get_idx_from_char(char c)
{
    c = toupper(c);
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }
    return -1; // Invalid input
}

int main()
{
    char sentence[128];  // Input message
    char key[128];       // Cipher key

    get_sentence("Enter the message:", sentence, sizeof(sentence));
    get_sentence("Enter the key:", key, sizeof(key));

    printf("Encrypting...\n");
    sleep(1); // Pause for effect

    int j = 0; // Index for key characters

    for (int i = 0; i < strlen(sentence); i++)
    {
        char c = sentence[i];
        if (isalpha(c))
        {
            // Get shift value from current key character
            int idx = get_idx_from_char(key[j % strlen(key)]);
            printf("%c", shift_char(c, idx));
            j++; // Only advance key index if input char is a letter
        }
        else
        {
            printf("%c", c); // Print non-alphabet characters unchanged
        }
    }

    printf("\n");
    return 0;
}