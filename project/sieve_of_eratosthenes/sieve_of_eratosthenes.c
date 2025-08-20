#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    for (int i = 2; i < n; i++)
    {
        if (i == 2 || i == 3 || i == 5)
        {
            printf("%d ", i);
            continue;
        }

        if (i % 2 == 0 || i % 3 == 0 || i % 5 == 0)
        {
            continue;
        }

        printf("%d ", i);
    }

    printf("\n");
    return 0;
}