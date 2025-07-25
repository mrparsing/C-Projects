#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// Pretty prints the Sudoku grid
void print_sudoku(int grid[9][9])
{
    printf("+-------+-------+-------+\n");
    for (int i = 0; i < 9; i++)
    {
        printf("| ");
        for (int j = 0; j < 9; j++)
        {
            if (grid[i][j] == 0)
                printf(". ");
            else
                printf("%d ", grid[i][j]);

            if ((j + 1) % 3 == 0)
                printf("| ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0)
            printf("+-------+-------+-------+\n");
    }
}

// Shuffles an array in place
void shuffle(int *array, int size)
{
    for (int i = 0; i < size; i++)
    {
        int j = rand() % size;
        int tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

// Checks if placing a number is valid
int is_safe(int grid[9][9], int row, int col, int num)
{
    for (int i = 0; i < 9; i++)
    {
        // Check row and column
        if (grid[row][i] == num || grid[i][col] == num)
            return 0;
    }

    // Check 3x3 box
    int start_row = row - row % 3;
    int start_col = col - col % 3;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[start_row + i][start_col + j] == num)
                return 0;

    return 1;
}

// Recursively fills the grid to create a complete valid Sudoku
int fill_grid(int grid[9][9])
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (grid[row][col] == 0)
            {
                int nums[9];
                for (int i = 0; i < 9; i++)
                    nums[i] = i + 1;

                shuffle(nums, 9);

                for (int i = 0; i < 9; i++)
                {
                    int num = nums[i];
                    if (is_safe(grid, row, col, num))
                    {
                        grid[row][col] = num;
                        if (fill_grid(grid))
                            return 1;
                        grid[row][col] = 0;
                    }
                }
                return 0;
            }
        }
    }
    return 1;
}

// Initializes an empty grid and fills it
void init_grid(int grid[9][9])
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            grid[i][j] = 0;

    fill_grid(grid);
}

// Helper function to count the number of solutions using backtracking
int count_solutions_helper(int grid[9][9], int *count, int limit)
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (grid[row][col] == 0)
            {
                for (int num = 1; num <= 9; num++)
                {
                    if (is_safe(grid, row, col, num))
                    {
                        grid[row][col] = num;
                        count_solutions_helper(grid, count, limit);
                        grid[row][col] = 0;

                        if (*count >= limit)
                            return *count;
                    }
                }
                return *count;
            }
        }
    }

    (*count)++;
    return *count;
}

// Wrapper to count the number of solutions of the current puzzle
int count_solutions(int grid[9][9])
{
    int count = 0;
    count_solutions_helper(grid, &count, 2); // stop as soon as 2 solutions are found
    return count;
}

// Removes cells while preserving unique solution
void dig_holes(int grid[9][9], int holes)
{
    while (holes > 0)
    {
        int row = rand() % 9;
        int col = rand() % 9;

        if (grid[row][col] == 0)
            continue; // already empty

        int backup = grid[row][col];
        grid[row][col] = 0;

        int copy[9][9];
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                copy[i][j] = grid[i][j];

        if (count_solutions(copy) != 1)
            grid[row][col] = backup; // restore if not unique
        else
            holes--;
    }
}

// Solves the Sudoku puzzle using backtracking
int solve_grid(int grid[9][9])
{
    for (int row = 0; row < 9; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (grid[row][col] == 0)
            {
                for (int num = 1; num <= 9; num++)
                {
                    if (is_safe(grid, row, col, num))
                    {
                        grid[row][col] = num;
                        if (solve_grid(grid))
                            return 1;
                        grid[row][col] = 0;
                    }
                }
                return 0;
            }
        }
    }
    return 1;
}

// Main function: generates, prints, and solves Sudoku
int main(int argc, char *argv[])
{
    srand(time(NULL));
    int grid[9][9];
    int holes;

    // Difficulty level from command line argument
    if (argc != 2)
    {
        printf("Usage: %s <difficulty>\n", argv[0]);
        printf("Difficulty: 1 = easy, 2 = medium, 3 = hard\n");
        return 1;
    }

    int difficulty = atoi(argv[1]);

    if (difficulty < 1 || difficulty > 3)
    {
        printf("Error: difficulty must be 1 (easy), 2 (medium), or 3 (hard)\n");
        return 1;
    }

    // Set number of holes based on difficulty
    switch (difficulty)
    {
    case 1: holes = 35; break; // easy
    case 2: holes = 45; break; // medium
    case 3: holes = 55; break; // hard
    }

    init_grid(grid);        // generate full grid
    dig_holes(grid, holes); // dig holes based on difficulty

    printf("Generated Sudoku (difficulty %d):\n\n", difficulty);
    print_sudoku(grid);

    // Make a copy of the puzzle to solve
    int solution[9][9];
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            solution[i][j] = grid[i][j];

    // Solve and print solution
    if (solve_grid(solution))
    {
        printf("\nSolution:\n\n");
        print_sudoku(solution);
    }
    else
    {
        printf("\nError: Sudoku could not be solved\n");
    }

    return 0;
}