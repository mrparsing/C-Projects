#include <stdio.h>
#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 20
#define NUM_CELL (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)
#define ROWS HEIGHT / CELL_SIZE
#define COLUMNS WIDTH / CELL_SIZE

#define COLOR_YELLOW 0xFFFF00
#define COLOR_BLUE 0x0000FF
#define COLOR_BLACK 0x000000
#define COLOR_GRAY 0x808080

struct Cell
{
    int x;    // column index
    int y;    // row index
    int type; // source = 1, goal = 2
};

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

void initialize_environment(struct Cell world[NUM_CELL])
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            world[j + COLUMNS * i] = (struct Cell){j, i, 0};
        }
    }
}

void draw_cell(SDL_Surface *surface, struct Cell cell, Uint32 color)
{
    SDL_Rect rect;
    rect.x = cell.x * CELL_SIZE;
    rect.y = cell.y * CELL_SIZE;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;

    SDL_FillRect(surface, &rect, color);
}

void draw_environment(SDL_Surface *surface, struct Cell environemnt[NUM_CELL])
{
    for (int i = 0; i < NUM_CELL; i++)
    {
        if (environemnt[i].type == 1)
            draw_cell(surface, environemnt[i], COLOR_YELLOW);
        else if (environemnt[i].type == 2)
            draw_cell(surface, environemnt[i], COLOR_BLUE);
        else
            draw_cell(surface, environemnt[i], COLOR_BLACK);
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
        "A* visualization",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    int running = 1;
    SDL_Event event;

    int cell_type = 0;

    struct Cell world[NUM_CELL];
    Uint32 color_yellow = SDL_MapRGB(surface->format, 255, 255, 0);
    Uint32 color_blue = SDL_MapRGB(surface->format, 0, 0, 255);
    Uint32 color_black = SDL_MapRGB(surface->format, 0, 0, 0);
    Uint32 color_gray = SDL_MapRGB(surface->format, 128, 128, 128);

    initialize_environment(world);
    while (running)
    {
        // Event handling
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;

            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_s)
                    cell_type = 1;
                else if (event.key.keysym.sym == SDLK_g)
                    cell_type = 2;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouse_cell_j = event.button.x / CELL_SIZE;
                int mouse_cell_i = event.button.y / CELL_SIZE;
                int index = mouse_cell_j + mouse_cell_i * COLUMNS;

                if (index >= 0 && index < NUM_CELL)
                    world[index].type = cell_type;
                printf("%d\n", cell_type);
            }
        }

        SDL_FillRect(surface, NULL, color_black); // Pulisce lo sfondo
        draw_environment(surface, world);
        draw_grid(surface, color_gray);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
