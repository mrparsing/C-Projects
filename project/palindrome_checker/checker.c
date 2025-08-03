#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <string>\n", argv[0]);
        return 1;
    }

    char *string = argv[1];

    int len = strlen(string);

    for (int i = 0; i < len; i++)
    {
        if (tolower(string[i]) != tolower(string[len - i - 1]))
        {
            printf("String not palindrome\n");
            return 1;
        } else {
            continue;
        }
    }
    printf("String palindrome\n");
    return 0;
}