#include <stdio.h>
#include <ctype.h>

int main()
{
    char filename[100];
    printf("Enter the file name: \n");
    scanf("%99s", filename);

    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("Could not open file\n");
        return 0;
    }
    else
    {
        int x;
        int word_counter = 0;
        int char_counter = 0;
        int in_word = 0;

        while ((x = fgetc(file)) != EOF)
        {
            if (!isspace(x))
            {
                char_counter++;
                if (!in_word)
                {
                    word_counter++;
                    in_word = 1;
                }
            }
            else
            {
                in_word = 0;
            }
        }

        fclose(file);

        printf("Characters: %d\n", char_counter);
        printf("Word: %d\n", word_counter);
    }
}