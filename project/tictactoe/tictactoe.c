#include <stdio.h>

void print(char grid[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        printf("-------------\n");
        printf("|");
        for (int j = 0; j < 3; j++)
        {
            printf(" %c |", grid[i][j]);
        }
        printf("\n");
    }
    printf("-------------\n");
}

int main()
{
    char grid[3][3] = {
    {'O', 'O', 'O'},
    {'O', 'O', 'O'},
    {'O', 'O', 'O'}
};
    print(grid);
}