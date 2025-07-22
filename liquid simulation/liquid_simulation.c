#include <stdio.h>
#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 45
#define WATER_TYPE 0
#define SOLID_TYPE 1
#define NUM_CELL (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)
#define ROWS HEIGHT / CELL_SIZE
#define COLUMNS WIDTH / CELL_SIZE

struct Cell
{
    int type;
    double fill_level;
    int x;
    int y;
};

struct CellFlow
{
    double flow_left;
    double flow_right;
    double flow_up;
    double flow_down;
};

void draw_grid(SDL_Surface *surface, Uint32 color)
{
    SDL_Rect line;

    // Linee verticali
    for (int x = 0; x <= WIDTH; x += CELL_SIZE)
    {
        line.x = x;
        line.y = 0;
        line.w = 1;
        line.h = HEIGHT;
        SDL_FillRect(surface, &line, color);
    }

    // Linee orizzontali
    for (int y = 0; y <= HEIGHT; y += CELL_SIZE)
    {
        line.x = 0;
        line.y = y;
        line.w = WIDTH;
        line.h = 1;
        SDL_FillRect(surface, &line, color);
    }
}

void draw_cell(SDL_Surface *surface, struct Cell cell)
{
    SDL_Rect rect;
    rect.x = cell.x * CELL_SIZE;
    rect.y = cell.y * CELL_SIZE;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;

    Uint32 color = SDL_MapRGB(surface->format, 0, 0, 0);
    SDL_FillRect(surface, &rect, color);

    if (cell.type == WATER_TYPE)
    {
        int water_height = cell.fill_level * CELL_SIZE;
        int empty_height = CELL_SIZE - water_height;
        SDL_Rect water = {rect.x, rect.y + empty_height, CELL_SIZE, water_height};
        color = SDL_MapRGB(surface->format, 105, 216, 255);
        SDL_FillRect(surface, &water, color);
    }
    if (cell.type == SOLID_TYPE)
    {
        color = SDL_MapRGB(surface->format, 255, 255, 255);
        SDL_FillRect(surface, &rect, color);
    }
}

void initialize_environment(struct Cell environment[NUM_CELL])
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            environment[j + COLUMNS * i] = (struct Cell){WATER_TYPE, 0, j, i};
        }
    }
}

void draw_environment(SDL_Surface *surface, struct Cell environemnt[NUM_CELL])
{
    for (int i = 0; i < NUM_CELL; i++)
    {
        draw_cell(surface, environemnt[i]);
    }
}

void simulation(struct Cell environemnt[NUM_CELL])
{
    struct CellFlow flows[NUM_CELL];

    for (int i = 0; i < NUM_CELL; i++)
    {
        flows[i] = (struct CellFlow){0, 0, 0, 0};
    }

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            int idx = j + COLUMNS * i;
            struct Cell current_cell = environemnt[idx];

            if (current_cell.type == SOLID_TYPE)
                continue;
            if (current_cell.fill_level <= 0.0)
                continue;

            if (i < ROWS - 1)
            {
                int down_idx = j + COLUMNS * (i + 1);
                if (environemnt[down_idx].type != SOLID_TYPE)
                {
                    double space = 1.0 - environemnt[down_idx].fill_level;
                    if (space > 0.0)
                    {
                        double amount = (current_cell.fill_level < space) ? current_cell.fill_level : space;
                        flows[idx].flow_down += amount;
                        flows[down_idx].flow_up += amount;
                    }
                }
            }
        }
    }

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            int idx = j + COLUMNS * i;
            environemnt[idx].fill_level += flows[idx].flow_up - flows[idx].flow_down;
            if (environemnt[idx].fill_level < 0.0)
                environemnt[idx].fill_level = 0.0;
            if (environemnt[idx].fill_level > 1.0)
                environemnt[idx].fill_level = 1.0;
        }
    }
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

    // Mappa i colori nel formato della surface
    Uint32 color_gray = SDL_MapRGB(surface->format, 130, 130, 130);
    Uint32 color_black = SDL_MapRGB(surface->format, 0, 0, 0);
    Uint32 color_white = SDL_MapRGB(surface->format, 255, 255, 255);

    int running = 1;
    SDL_Event e;

    int current_type = SOLID_TYPE;
    int delete_mode = 0;

    struct Cell environment[NUM_CELL];
    initialize_environment(environment);

    while (running)
    {
        // Cancella tutto con sfondo nero
        // SDL_FillRect(surface, NULL, color_black);

        // Gestione eventi
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }

            if (e.type == SDL_MOUSEMOTION)
            {
                if (e.motion.state != 0)
                {
                    int mouse_cell_j = e.motion.x / CELL_SIZE;
                    int mouse_cell_i = e.motion.y / CELL_SIZE;
                    int fill_level = delete_mode ? 0 : 1;
                    if (delete_mode != 0)
                    {
                        current_type = WATER_TYPE;
                    }
                    struct Cell cell = {current_type, fill_level, mouse_cell_j, mouse_cell_i};
                    environment[mouse_cell_j + COLUMNS * mouse_cell_i] = cell;
                }
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
            {
                current_type = 1 - current_type;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE)
            {
                delete_mode = 1 - delete_mode;
            }
        }

        // SIMULATION
        simulation(environment);

        draw_environment(surface, environment);
        draw_grid(surface, color_gray);
        // Aggiorna il contenuto della finestra
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}