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

int check_row(char grid[3][3], char player, int row)
{
    for (int col = 0; col < 3; col++)
    {
        if (grid[row][col] != player)
            return 0;
    }
    return 1;
}

int check_col(char grid[3][3], char player, int col)
{
    for (int row = 0; row < 3; row++)
    {
        if (grid[row][col] != player)
            return 0;
    }
    return 1;
}

int check_diagonal(char grid[3][3], char player)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i == j && grid[i][j] != player)
                return 0;
        }
    }
    return 1;
}

int check_anti_diagonal(char grid[3][3], char player)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if ((2 - i) == j && grid[i][j] != player)
                return 0;
        }
    }
    return 1;
}

int check_win(char grid[3][3], char player)
{
    for (int i = 0; i < 3; i++)
    {
        if (check_row(grid, player, i))
            return 1;
    }
    for (int i = 0; i < 3; i++)
    {
        if (check_col(grid, player, i))
            return 1;
    }
    if (check_diagonal(grid, player))
        return 1;
    if (check_anti_diagonal(grid, player))
        return 1;
    return 0;
}

int main()
{
    char grid[3][3] = {
        {' ', ' ', ' '},
        {' ', ' ', ' '},
        {' ', ' ', ' '}};
    printf("Welcome to tictactoe game!\nYou are X, computer is O\n");
    printf("Choose a position from 1 to 9\n");
    print(grid);
    printf("%d\n", check_win(grid, 'X'));
}