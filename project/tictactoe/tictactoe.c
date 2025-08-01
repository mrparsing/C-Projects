#include <stdio.h>
#include <limits.h>

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
        if (grid[i][i] != player)
            return 0;
    }
    return 1;
}

int check_anti_diagonal(char grid[3][3], char player)
{
    for (int i = 0; i < 3; i++)
    {
        if (grid[i][2 - i] != player)
            return 0;
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

int full_grid(char grid[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (grid[i][j] == ' ')
            {
                return 0;
            }
        }
    }
    return 1;
}

int minimax(char grid[3][3], char player)
{
    if (check_win(grid, 'X'))
        return -10;
    if (check_win(grid, 'O'))
        return 10;
    if (full_grid(grid))
        return 0;

    if (player == 'O') {
        int best_val = -INT_MAX;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (grid[i][j] == ' ') {
                    grid[i][j] = 'O';
                    int val = minimax(grid, 'X');
                    grid[i][j] = ' ';
                    if (val > best_val) 
                        best_val = val;
                }
            }
        }
        return best_val;
    } else {
        int best_val = INT_MAX;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (grid[i][j] == ' ') {
                    grid[i][j] = 'X';
                    int val = minimax(grid, 'O');
                    grid[i][j] = ' ';
                    if (val < best_val) 
                        best_val = val;
                }
            }
        }
        return best_val;
    }
}

int findBestMove(char grid[3][3]) {
    int best_val = -INT_MAX;
    int best_move = -1;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i][j] == ' ') {
                grid[i][j] = 'O';
                int move_val = minimax(grid, 'X');
                grid[i][j] = ' ';
                if (move_val > best_val) {
                    best_val = move_val;
                    best_move = i * 3 + j;
                }
            }
        }
    }
    return best_move;
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

    while (1)
    {
        int pos = -1;
        printf("Enter a position: ");
        scanf("%d", &pos);

        if (pos < 1 || pos > 9)
        {
            printf("Position not valid, try again :(\n");
            continue;
        }

        int row = (pos - 1) / 3;
        int col = (pos - 1) % 3;

        if (grid[row][col] != ' ')
        {
            printf("Position already occupied, try again :(\n");
            continue;
        }

        grid[row][col] = 'X';

        print(grid);

        if (check_win(grid, 'X'))
        {
            printf("Congratulation! You won!!\n");
            break;
        }
        if (full_grid(grid))
        {
            printf("Draw!");
            break;
        }

        int pc_move = findBestMove(grid);
        row = pc_move / 3;
        col = pc_move % 3;
        grid[row][col] = 'O';

        print(grid);

        if (check_win(grid, 'O'))
        {
            printf("PC won! Try again\n");
            break;
        }
        if (full_grid(grid))
        {
            printf("Draw!1n");
            break;
        }
    }
}