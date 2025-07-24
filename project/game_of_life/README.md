# Game of Life

Implementation of **Conway’s Game of Life** using an `SDL_Surface` for drawing. Cells live in a 2D array (flattened to 1D), evolve each frame per the classic rules, and you can interact by pausing, stepping, resetting, and painting live cells with the mouse.

---

# Demo

![Demo](../assets/game_of_life.gif)

## 1. Constants & Dimensions

```c
#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 15
#define NUM_CELL ((WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE))
#define ROWS (HEIGHT / CELL_SIZE)
#define COLUMNS (WIDTH / CELL_SIZE)
```

* **WIDTH/HEIGHT**: window size in pixels.
* **CELL\_SIZE**: each cell rendered as a `CELL_SIZE`×`CELL_SIZE` rectangle.
* **ROWS/COLUMNS**: logical grid size; width/height divided by cell size.
* **NUM\_CELL**: total cells = ROWS × COLUMNS.

Color macros use packed ARGB/ABGR integers (SDL maps them anyway):

```c
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xffffffff
```

---

## 2. Cell Structure

```c
struct Cell {
    int x;     // column index
    int y;     // row index
    int live;  // 1 = alive, 0 = dead
};
```

Each cell stores its grid coordinates (redundant but handy for drawing) and a boolean `live` flag.

---

## 3. Drawing Utilities

### 3.1 Grid Overlay

```c
void draw_grid(SDL_Surface *surface, Uint32 color) { ... }
```

Draws vertical lines every `CELL_SIZE` pixels (x-axis) and horizontal lines (y-axis). Purely cosmetic.

### 3.2 Single Cell

```c
void draw_cell(SDL_Surface *surface, struct Cell cellData, int live) {
    SDL_Rect rect = { cellData.x * CELL_SIZE, cellData.y * CELL_SIZE,
                      CELL_SIZE, CELL_SIZE };
    Uint32 color = live ? SDL_MapRGB(surface->format, 255, 255, 255)
                        : SDL_MapRGB(surface->format,   0,   0,   0);
    SDL_FillRect(surface, &rect, color);
}
```

White = live, black = dead. (Background clear + dead painting is redundant but fine.)

### 3.3 Whole World

```c
void draw_environment(SDL_Surface *surface, struct Cell world[NUM_CELL]) {
    for (int i = 0; i < NUM_CELL; i++)
        draw_cell(surface, world[i], world[i].live);
}
```

This version draws all cells (even dead). In `main` you actually redraw only alive cells for speed, then overlay the grid.

---

## 4. Initialization

```c
void initialize_environment(struct Cell world[NUM_CELL]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            double r = (double)rand() / RAND_MAX;
            world[j + COLUMNS * i] = (struct Cell){ j, i, r > 0.6 }; // ~40% alive
        }
    }
}
```

---

## 5. Update Step (Conway’s Rules)

```c
void check_cell(struct Cell world[NUM_CELL]) {
    struct Cell world_copy[NUM_CELL];
    memcpy(world_copy, world, sizeof(world_copy));

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            struct Cell cur = world_copy[j + COLUMNS * i];
            int live = 0;
            // count 8 neighbors (with boundary checks)
            if (i > 0 && j > 0)                 live += world_copy[(j-1) + COLUMNS*(i-1)].live;
            if (i > 0)                          live += world_copy[j     + COLUMNS*(i-1)].live;
            if (i > 0 && j < COLUMNS-1)         live += world_copy[(j+1) + COLUMNS*(i-1)].live;
            if (j > 0)                          live += world_copy[(j-1) + COLUMNS*i    ].live;
            if (j < COLUMNS-1)                  live += world_copy[(j+1) + COLUMNS*i    ].live;
            if (j > 0 && i < ROWS-1)            live += world_copy[(j-1) + COLUMNS*(i+1)].live;
            if (i < ROWS-1)                     live += world_copy[j     + COLUMNS*(i+1)].live;
            if (j < COLUMNS-1 && i < ROWS-1)    live += world_copy[(j+1) + COLUMNS*(i+1)].live;

            // Conway's rules
            if (cur.live && (live < 2 || live > 3))      world[j + COLUMNS*i].live = 0; // dies
            else if (!cur.live && live == 3)             world[j + COLUMNS*i].live = 1; // born
            else                                         world[j + COLUMNS*i].live = cur.live; // unchanged
        }
    }
}
```

Key points:

* Work on a copy so updates don’t cascade in the same generation.
* Eight neighbor checks with explicit bounds.
* Apply rules verbatim:

  * **Underpopulation**: live < 2 → dies
  * **Overcrowding**: live > 3 → dies
  * **Reproduction**: dead & live == 3 → becomes live
  * Else: stays the same

---

## 6. Main Loop & Controls

```c
int running = 1;
int paused  = 1;   // start paused
...
while (running) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) running = 0;
        if (event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.sym) {
            case SDLK_SPACE: paused = !paused; break;  // pause/resume
            case SDLK_n: if (paused) check_cell(world); break; // step once
            case SDLK_r: initialize_environment(world); break; // new random board
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x = event.button.x / CELL_SIZE;
            int y = event.button.y / CELL_SIZE;
            world[x + COLUMNS*y] = (struct Cell){x,y,1}; // paint live cell
        }
    }

    // Clear & render alive cells only
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
    for (int i = 0; i < NUM_CELL; ++i)
        if (world[i].live)
            draw_cell(surface, world[i], 1);

    draw_grid(surface, color_gray); // optional overlay
    SDL_UpdateWindowSurface(window);

    if (!paused)
        check_cell(world); // evolve one generation

    SDL_Delay(16); // ~60 FPS cap
}
```

### Keyboard

* **Space**: toggle pause/play.
* **N**: step one generation (only works when paused, by design).
* **R**: reset to a new random pattern.

### Mouse

* Left click sets that cell alive (no drag handling here, but you can add it). You can implement toggling or drag painting easily.