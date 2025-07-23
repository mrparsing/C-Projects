# 2D Cellular Water Simulation

This program simulates water flowing through a grid of square cells using a very simple cellular model. Each cell stores **type** (water/solid) and **fill level** (0 → empty, 1 → full). Gravity pulls water down; if it can’t go down, it spreads sideways; if a cell overfills, excess is pushed upward. The result is a quick-and-dirty fluid toy you can paint with the mouse.

---

# Demo

![Demo](../assets/liquid%20simulation.gif)

---

## 1. Constants & Macros

```c
#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 15
#define WATER_TYPE 0
#define SOLID_TYPE 1
#define NUM_CELL (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)
#define ROWS HEIGHT / CELL_SIZE
#define COLUMNS WIDTH / CELL_SIZE
```

* **WIDTH/HEIGHT**: window size in pixels.
* **CELL\_SIZE**: each grid cell is a square of 15×15 px.
* **ROWS / COLUMNS**: logical grid dimensions.
* **NUM\_CELL**: total number of cells (ROWS × COLUMNS).
* **WATER\_TYPE / SOLID\_TYPE**: two possible cell types.

### Helper macros

```c
static inline int idx(int x, int y) { return x + y * COLUMNS; }
static inline double clampd(double v, double lo, double hi) { ... }
```

* `idx(x,y)`: converts (column, row) into linear index for 1D arrays.
* `clampd`: ensures a double stays within `[lo, hi]`.

---

## 2. Data Structures

```c
struct Cell {
    int type;          // WATER_TYPE or SOLID_TYPE
    double fill_level; // 0.0 .. 1.0 (fraction of the cell filled with water)
    int x, y;          // integer indices in the grid (column, row)
    int flowing_down;  // currently unused, reserved for future logic
};
```

Each cell knows what it is, how much water it contains, and its coordinates. Coordinates are redundant (they can be derived from the index) but convenient when drawing.

```c
struct CellFlow {
    double flow_left, flow_right, flow_up, flow_down;
};
```

This struct is declared but not used in the present logic—handy if you later move to a more physically motivated flow model (e.g., based on pressures between neighbors).

---

## 3. Rendering

### 3.1 Grid lines

```c
void draw_grid(SDL_Surface *surface, Uint32 color) { ... }
```

Loops over X and Y to draw 1‑pixel lines every `CELL_SIZE` pixels. Purely cosmetic to visualize the discrete cells.

### 3.2 Single cell

```c
void draw_cell(SDL_Surface *surface, struct Cell cellData) { ... }
```

1. Compute pixel rectangle for the cell.
2. Fill white background.
3. If **water**: compute a blue rectangle whose height is proportional to `fill_level` (0 → empty, 1 → full). Draw it aligned to the bottom of the cell.
4. If **solid**: fill the whole cell black.

### 3.3 Entire environment

```c
void draw_environment(SDL_Surface *surface, struct Cell environment[NUM_CELL]) { ... }
```

* Iterates all cells and calls `draw_cell`.
* Then does an extra pass: if two vertically adjacent water cells are both fairly full (>0.6), it redraws the lower one. (Visual tweak; doesn’t affect physics.)

---

## 4. Simulation Steps

The simulation is split into three passes per frame:

1. **Gravity** – let water move down.
2. **Horizontal spreading** – if it can’t go down, spread left/right.
3. **Upward push** – if overfilled (>1.0), push excess up.

Everything works on copies (`grid_next`) to avoid order‑of‑update artifacts.

### 4.1 Gravity

```c
void simulation_gravity(struct Cell grid[NUM_CELL]) {
    struct Cell grid_next[NUM_CELL];
    memcpy(grid_next, grid, sizeof(grid_next));

    for (int y = ROWS - 2; y >= 0; --y) {
        for (int x = 0; x < COLUMNS; ++x) {
            int src_i = idx(x, y);
            int dst_i = idx(x, y + 1);
            ...
            if (src.type == WATER_TYPE && src.fill_level > 0.0 && dst.type != SOLID_TYPE) {
                double free_space = 1.0 - dst.fill_level;
                if (free_space > 0.0) {
                    double transfer = fmin(src.fill_level, free_space);
                    grid_next[src_i].fill_level -= transfer;
                    grid_next[dst_i].fill_level += transfer;
                }
            }
        }
    }
    // Clamp and copy back
    for (int i = 0; i < NUM_CELL; ++i) {
        grid_next[i].fill_level = clampd(grid_next[i].fill_level, 0.0, 1.0);
        grid[i] = grid_next[i];
    }
}
```

* Iterate bottom‑up so upper cells “see” the previous frame’s values below them.
* Compute the free space in the destination cell; transfer as much as possible up to the source amount.
* Final pass clamps the fill levels.

### 4.2 Horizontal spreading

```c
void spreading_water(struct Cell grid[NUM_CELL]) {
    struct Cell grid_next[NUM_CELL];
    for (int i = 0; i < NUM_CELL; i++) grid_next[i] = grid[i];

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            if (i + 1 == ROWS ||
                grid[idx(j, i + 1)].fill_level >= grid[idx(j, i)].fill_level ||
                grid[idx(j, i + 1)].type == SOLID_TYPE) {
                // We only spread if we're "resting" on something (bottom, solid, or water level below >= current).
                struct Cell src = grid[idx(j, i)];
                if (src.type == WATER_TYPE) {
                    // LEFT
                    if (j > 0) {
                        struct Cell dst = grid[idx(j - 1, i)];
                        if (dst.type == WATER_TYPE && dst.fill_level < src.fill_level) {
                            double delta = src.fill_level - dst.fill_level;
                            grid_next[idx(j, i)].fill_level     -= delta / 3.0;
                            grid_next[idx(j - 1, i)].fill_level += delta / 3.0;
                        }
                    }
                    // RIGHT
                    if (j < COLUMNS - 1) {
                        struct Cell dst = grid[idx(j + 1, i)];
                        if (dst.fill_level < src.fill_level) {
                            double delta = src.fill_level - dst.fill_level;
                            grid_next[idx(j, i)].fill_level     -= delta / 3.0;
                            grid_next[idx(j + 1, i)].fill_level += delta / 3.0;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < NUM_CELL; i++) grid[i] = grid_next[i];
}
```

* Only spread when the cell isn’t actively pouring downward (heuristic check using cell below).
* Moves 1/3 of the difference to each side (arbitrary factor to keep motion mild).

### 4.3 Upward push

```c
void upwards_water(struct Cell grid[NUM_CELL]) {
    struct Cell grid_next[NUM_CELL];
    for (int i = 0; i < NUM_CELL; i++) grid_next[i] = grid[i];

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            struct Cell src = grid[idx(j, i)];
            if (src.type == WATER_TYPE && src.fill_level > 1.0 && i > 0) {
                struct Cell above = grid[idx(j, i - 1)];
                if (above.type == WATER_TYPE && src.fill_level > above.fill_level) {
                    double transfer = src.fill_level - 1.0; // push just the excess
                    grid_next[idx(j, i)].fill_level     -= transfer;
                    grid_next[idx(j, i - 1)].fill_level += transfer;
                }
            }
        }
    }

    for (int i = 0; i < NUM_CELL; i++) grid[i] = grid_next[i];
}
```

* If a cell gets over 1.0 ("physically" overfilled), push the excess up if that cell is also water.

### 4.4 Combine steps

```c
void simulation(struct Cell grid[NUM_CELL]) {
    simulation_gravity(grid);
    spreading_water(grid);
    upwards_water(grid);
}
```

One call per frame. Order matters: gravity first, then lateral spread, then overflow correction.

---

## 5. Input Handling & Painting

Inside the main event loop:

* **Mouse drag** (`SDL_MOUSEMOTION` with button down): paints cells.

  * If **erase\_mode** is on, set cell to water with `fill_level = 0`.
  * Otherwise, toggle between SOLID and WATER cells with increasing fill.
* **SPACE** toggles `active_type` (solid ↔ water).
* **BACKSPACE** toggles `erase_mode` (erase vs. add).

```c
if (event.type == SDL_MOUSEMOTION) {
    if (event.motion.state != 0) {
        int j = event.motion.x / CELL_SIZE;
        int i = event.motion.y / CELL_SIZE;
        int lvl;
        struct Cell new_cell;

        if (erase_mode) {
            active_type = WATER_TYPE;
            lvl = 0;
            new_cell = (struct Cell){active_type, lvl, j, i};
        } else {
            lvl = world[idx(j, i)].fill_level + 1;
            new_cell = (struct Cell){active_type, lvl, j, i};
        }
        world[idx(j, i)] = new_cell;
    }
}
```

---

## 6. Main Loop Order

1. **Events** → update world based on input.
2. **Simulation** → one step of gravity/spread/upwards.
3. **Rendering** → draw cells, then grid lines.
4. **SDL\_UpdateWindowSurface** & a short delay.

```c
while (running) {
    while (SDL_PollEvent(&event)) { ... }
    simulation(world);
    draw_environment(surface, world);
    draw_grid(surface, color_gray);
    SDL_UpdateWindowSurface(window);
    SDL_Delay(10);
}
```

---

## 7. Tuning & Notes

* **CELL\_SIZE** affects resolution: smaller = smoother water but slower.
* **Transfer factors** (`delta/3` etc.) are arbitrary. Adjust for faster/slower spread.
* **Order of passes** is crucial. Try swapping them to see how behavior changes.
* There’s no real conservation of mass if you keep painting water everywhere—this is a toy model.
* Performance: everything is O(ROWS×COLUMNS) each frame. Fine for small grids.