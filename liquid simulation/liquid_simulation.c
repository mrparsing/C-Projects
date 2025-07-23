#include <stdio.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 15
#define WATER_TYPE 0
#define SOLID_TYPE 1
#define NUM_CELL (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)
#define ROWS HEIGHT / CELL_SIZE
#define COLUMNS WIDTH / CELL_SIZE

// Single cell of the grid: holds what type it is and how much water is inside.
struct Cell
{
    int type;              // WATER_TYPE or SOLID_TYPE
    double fill_level;     // 0.0 (empty) .. 1.0 (full)
    int x;                 // column index
    int y;                 // row index
    int flowing_down;      // unused flag (kept for future features)
};

// Not used in current logic, kept for potential extension
struct CellFlow
{
    double flow_left;
    double flow_right;
    double flow_up;
    double flow_down;
};

// Convert (x, y) to linear index in the array
static inline int idx(int x, int y) { return x + y * COLUMNS; }

// Clamp a double between two bounds
static inline double clampd(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Draw the grid lines for visualization
void draw_grid(SDL_Surface *surface, Uint32 color)
{
    SDL_Rect line;

    // Vertical lines
    for (int x = 0; x <= WIDTH; x += CELL_SIZE)
    {
        line.x = x;
        line.y = 0;
        line.w = 1;
        line.h = HEIGHT;
        SDL_FillRect(surface, &line, color);
    }

    // Horizontal lines
    for (int y = 0; y <= HEIGHT; y += CELL_SIZE)
    {
        line.x = 0;
        line.y = y;
        line.w = WIDTH;
        line.h = 1;
        SDL_FillRect(surface, &line, color);
    }
}

// Render a single cell (background + water or solid)
void draw_cell(SDL_Surface *surface, struct Cell cellData)
{
    SDL_Rect rect;
    rect.x = cellData.x * CELL_SIZE;
    rect.y = cellData.y * CELL_SIZE;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;

    // White background
    Uint32 color = SDL_MapRGB(surface->format, 255, 255, 255);
    SDL_FillRect(surface, &rect, color);

    if (cellData.type == WATER_TYPE) {
        // Determine water height (scaled by fill level)
        int water_height = (cellData.fill_level >= 1.0) ? CELL_SIZE : (int)(cellData.fill_level * CELL_SIZE);
        if (water_height > 0) {
            int empty_height = CELL_SIZE - water_height;
            SDL_Rect water = { rect.x, rect.y + empty_height, CELL_SIZE, water_height };
            color = SDL_MapRGB(surface->format, 50, 200, 255);
            SDL_FillRect(surface, &water, color);
        }
    } else if (cellData.type == SOLID_TYPE) {
        // Solid blocks are black
        color = SDL_MapRGB(surface->format, 0, 0, 0);
        SDL_FillRect(surface, &rect, color);
    }
}

// Initialize every cell as empty water type
void initialize_environment(struct Cell world[NUM_CELL])
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            world[j + COLUMNS * i] = (struct Cell){WATER_TYPE, 0.0, j, i, 0};
        }
    }
}

// Draw the whole environment
void draw_environment(SDL_Surface *surface, struct Cell environemnt[NUM_CELL])
{
    for (int i = 0; i < NUM_CELL; i++)
    {
        draw_cell(surface, environemnt[i]);
    }

    // Extra pass for a small visual effect when two full water cells touch vertically
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            struct Cell cell_above = environemnt[j + COLUMNS * (i - 1)];
            struct Cell current_cell = environemnt[j + COLUMNS * i];
            if (i > 0 && cell_above.type == WATER_TYPE && cell_above.fill_level > 0.6 && current_cell.fill_level > 0.6 && current_cell.type == WATER_TYPE)
            {
                if (cell_above.fill_level > 0 && current_cell.fill_level > 0)
                {
                    draw_cell(surface, environemnt[j + COLUMNS * i]);
                }
            }
        }
    }
}

// Simple gravity step: move water down if there's free space
void simulation_gravity(struct Cell grid[NUM_CELL])
{
    struct Cell grid_next[NUM_CELL];
    memcpy(grid_next, grid, sizeof(grid_next));

    // From bottom-1 to top helps a bit with stability
    for (int y = ROWS - 2; y >= 0; --y) {
        for (int x = 0; x < COLUMNS; ++x) {
            int src_i = idx(x, y);
            int dst_i = idx(x, y + 1);

            struct Cell src = grid[src_i];
            struct Cell dst = grid[dst_i];

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

    // Final clamp
    for (int i = 0; i < NUM_CELL; ++i) {
        grid_next[i].fill_level = clampd(grid_next[i].fill_level, 0.0, 1.0);
        grid[i] = grid_next[i];
    }
}

// Spread water horizontally when it's not falling
void spreading_water(struct Cell grid[NUM_CELL])
{
    struct Cell grid_next[NUM_CELL];

    for (int i = 0; i < NUM_CELL; i++)
    {
        grid_next[i] = grid[i];
    }

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            if (i + 1 == ROWS || grid[j + COLUMNS * (i + 1)].fill_level >= grid[j + COLUMNS * i].fill_level || grid[j + COLUMNS * (i + 1)].type == SOLID_TYPE)
            {
                struct Cell src_cell = grid[j + COLUMNS * i];
                // LEFT
                if (src_cell.type == WATER_TYPE && j > 0)
                {
                    struct Cell dst_cell = grid[(j - 1) + COLUMNS * i];
                    if (dst_cell.type == WATER_TYPE && dst_cell.fill_level < src_cell.fill_level)
                    {
                        double delta_fill = src_cell.fill_level - dst_cell.fill_level;
                        grid_next[j + COLUMNS * i].fill_level -= delta_fill / 3;
                        grid_next[(j - 1) + COLUMNS * i].fill_level += delta_fill / 3;
                    }
                }
                // RIGHT
                if (src_cell.type == WATER_TYPE && j < COLUMNS - 1)
                {
                    struct Cell dst_cell = grid[(j + 1) + COLUMNS * i];
                    if (dst_cell.fill_level < src_cell.fill_level)
                    {
                        double delta_fill = src_cell.fill_level - dst_cell.fill_level;
                        grid_next[j + COLUMNS * i].fill_level -= delta_fill / 3;
                        grid_next[(j + 1) + COLUMNS * i].fill_level += delta_fill / 3;
                    }
                }
            }
        }
    }

    for (int i = 0; i < NUM_CELL; i++)
    {
        grid[i] = grid_next[i];
    }
}

// Push excess water upwards (when a cell is overfilled > 1.0)
void upwards_water(struct Cell grid[NUM_CELL])
{
    struct Cell grid_next[NUM_CELL];

    for (int i = 0; i < NUM_CELL; i++)
    {
        grid_next[i] = grid[i];
    }

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            struct Cell src_cell = grid[j + COLUMNS * i];
            if (src_cell.type == WATER_TYPE && src_cell.fill_level > 1 && i > 0 && grid[j + COLUMNS * (i - 1)].type == WATER_TYPE && src_cell.fill_level > grid[j + COLUMNS * (i - 1)].fill_level)
            {
                struct Cell dst_cell = grid[j + COLUMNS * (i - 1)];
                double transfer_fill = src_cell.fill_level - 1;
                grid_next[j + COLUMNS * i].fill_level -= transfer_fill;
                grid_next[j + COLUMNS * (i - 1)].fill_level += transfer_fill;
            }
        }
    }

    for (int i = 0; i < NUM_CELL; i++)
    {
        grid[i] = grid_next[i];
    }
}

// Single simulation step combining gravity + lateral spread + vertical push
void simulation(struct Cell grid[NUM_CELL])
{
    simulation_gravity(grid);
    spreading_water(grid);
    upwards_water(grid);
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Liquid simulation",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    // Map colors in the surface format
    Uint32 color_gray = SDL_MapRGB(surface->format, 130, 130, 130);
    Uint32 color_black = SDL_MapRGB(surface->format, 0, 0, 0);
    Uint32 color_white = SDL_MapRGB(surface->format, 255, 255, 255);

    int running = 1;
    SDL_Event event;            // renamed from 'e'

    int active_type = SOLID_TYPE;   // renamed from 'current_type'
    int erase_mode = 0;             // renamed from 'delete_mode'

    struct Cell world[NUM_CELL];    // renamed from 'environment'
    initialize_environment(world);

    while (running)
    {
        // Event handling
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }

            if (event.type == SDL_MOUSEMOTION)
            {
                if (event.motion.state != 0)
                {
                    int mouse_cell_j = event.motion.x / CELL_SIZE;
                    int mouse_cell_i = event.motion.y / CELL_SIZE;
                    int lvl;                     // renamed from 'fill_level'
                    struct Cell new_cell;        // renamed from 'cell'

                    if (erase_mode != 0)
                    {
                        active_type = WATER_TYPE;
                        lvl = 0;
                        new_cell = (struct Cell){active_type, lvl, mouse_cell_j, mouse_cell_i};
                    }
                    else
                    {
                        lvl = world[mouse_cell_j + COLUMNS * mouse_cell_i].fill_level + 1;
                        new_cell = (struct Cell){active_type, lvl, mouse_cell_j, mouse_cell_i};
                    }
                    world[mouse_cell_j + COLUMNS * mouse_cell_i] = new_cell;
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
            {
                active_type = 1 - active_type;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE)
            {
                erase_mode = 1 - erase_mode;
            }
        }

        // SIMULATION
        simulation(world);

        draw_environment(surface, world);
        draw_grid(surface, color_gray);
        // Update window contents
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
