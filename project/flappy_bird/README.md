# Flappy Bird in SDL2

This project implements a clone of the Flappy Bird game using the SDL2 library in C.

# Demo

![Demo](../assets/flappy%20bird.gif)

## Dependencies
- SDL2
- SDL2_ttf (for text rendering)
- SDL2_image (for loading textures)

## Code Structure

### 1. Initialization and Constants
```c
#define WIDTH 600
#define HEIGHT 600
#define NUM_COLUMNS 10
#define GRAVITY 800.0
```

### 2. Data Structures
**Column (Pipe):**
```c
typedef struct {
    double x;          // X position
    double gap_y;      // Vertical start of gap
    double gap_height; // Gap height
    double width;      // Column width
} Column;
```

**Bird:**
```c
typedef struct {
    double x;
    double y;
    double vy;
} Bird;
```

### 3. Main Game Logic

**Main Flow:**
1. Initialize SDL and load assets
2. Create window and renderer
3. Initialize columns and bird
4. Game loop:
   - Handle events (key presses)
   - Update game state
   - Render elements

**Key Functions:**
- `init_columns()`: Generates columns with random gaps
- `apply_gravity()`: Applies gravity to the bird
- `jump()`: Handles jumping when spacebar is pressed
- `move_columns()`: Moves and repositions columns
- `check_collision()`: Detects collisions with pipes and edges
- `render_text()`: Displays score and messages

### 4. Game Mechanics
- **Controls:**
  - `SPACE` to jump
  - `R` to restart after game over
  
- **Scoring:**
  +1 when the bird passes through a pipe

- **Game Over Conditions:**
  - Bird collides with a pipe
  - Bird goes off-screen (top/bottom)

### 5. Rendering
Rendered elements:
1. Background (from `background.png`)
2. Pipes (from `pipe.png`)
3. Bird (from `bird.png`)
4. Score text (using `FreeSansBold.otf`)

## Compilation and Execution

```bash
chmod +x build.sh
./build.sh
```

## Required Assets
Create a `textures` folder containing:
- `bird.png` (32x32px)
- `pipe.png` (pipe texture)
- `background.png` (600x600px background)
- `FreeSansBold.otf` (text font)