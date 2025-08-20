#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

void get_sentence(char *msg, char *buf, size_t size)
{
    printf("%s ", msg);
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0;
}

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
    return c;
}

int get_idx_from_char(char c)
{
    c = toupper(c);
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }
    return -1;
}

int main()
{
    char sentence[128];
    char key[128];

    get_sentence("Enter the message:", sentence, sizeof(sentence));
    get_sentence("Enter the key:", key, sizeof(key));

    printf("Encrypting...\n");
    sleep(1);

    int j = 0;

    for (int i = 0; i < strlen(sentence); i++)
    {
        char c = sentence[i];
        if (isalpha(c))
        {
            int idx = get_idx_from_char(key[j % strlen(key)]);
            printf("%c", shift_char(c, idx));
            j++;
        }
        else
        {
            printf("%c", c);
        }
    }

    printf("\n");
    return 0;
}