# Tic Tac Toe

This program implements a classic Tic Tac Toe game where you play against an AI opponent that uses the minimax algorithm to make optimal moves. The game runs in the console and features a smart computer player that's difficult to beat!

## Compile

1. Save the code in a file named `tictactoe.c`
2. Compile with GCC:
   ```chmod +x build.sh
   ./build
   ```
3. Run the executable:
   ```bash
   ./tictactoe
   ```

## Game Features

- **Player vs Computer**: You play as 'X' and the computer plays as 'O'
- **Smart AI**: Computer uses the minimax algorithm to make optimal moves
- **Win Detection**: Automatically detects wins for either player
- **Draw Detection**: Recognizes when the game ends in a tie
- **Input Validation**: Checks for valid moves and occupied positions

## Gameplay Instructions

1. The game board positions are numbered 1-9:
   ```
     1 | 2 | 3
     4 | 5 | 6
     7 | 8 | 9
   ```
2. On your turn, enter a number (1-9) to place your 'X' in that position
3. The computer will automatically respond with its 'O' move
4. The game ends when either:
   - A player gets three in a row (horizontally, vertically, or diagonally)
   - The board is full (draw)

## Key Functions

### `minimax(char grid[3][3], char player)`
- Implements the minimax algorithm to evaluate board positions
- Returns a score representing the game state:
  - `10`: Computer ('O') wins
  - `-10`: Player ('X') wins
  - `0`: Draw or neutral position
- Recursively evaluates all possible moves to find the optimal play

### `findBestMove(char grid[3][3])`
- Uses the minimax scores to determine the computer's best move
- Iterates through all empty positions
- Selects the move with the highest minimax score for the computer

### `check_win(char grid[3][3], char player)`
- Checks all possible win conditions:
  - Horizontal rows
  - Vertical columns
  - Main diagonal
  - Anti-diagonal
- Returns 1 if the specified player has won, 0 otherwise

### `full_grid(char grid[3][3])`
- Checks if the board is completely filled
- Returns 1 if full (draw), 0 otherwise

### `print(char grid[3][3])`
- Displays the current game board in a user-friendly format
- Shows player moves and board boundaries