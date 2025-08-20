#include <stdio.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 15
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xffffffff
#define NUM_CELL ((WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE))
#define ROWS (HEIGHT / CELL_SIZE)
#define COLUMNS (WIDTH / CELL_SIZE)

struct Cell {
    int x;
    int y;
    int live;
};

void draw_grid(SDL_Surface *surface, Uint32 color) {
    SDL_Rect line;

    for (int x = 0; x <= WIDTH; x += CELL_SIZE) {
        line.x = x;
        line.y = 0;
        line.w = 1;
        line.h = HEIGHT;
        SDL_FillRect(surface, &line, color);
    }

    for (int y = 0; y <= HEIGHT; y += CELL_SIZE) {
        line.x = 0;
        line.y = y;
        line.w = WIDTH;
        line.h = 1;
        SDL_FillRect(surface, &line, color);
    }
}

void initialize_environment(struct Cell world[NUM_CELL]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            double r = (double)rand() / (double)RAND_MAX;
            if (r > 0.6)
                world[j + COLUMNS * i] = (struct Cell){j, i, 1};
            else
                world[j + COLUMNS * i] = (struct Cell){j, i, 0};
        }
    }
}

void draw_cell(SDL_Surface *surface, struct Cell cellData, int live) {
    SDL_Rect rect;
    rect.x = cellData.x * CELL_SIZE;
    rect.y = cellData.y * CELL_SIZE;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;

    Uint32 color = (live == 1) ?
        SDL_MapRGB(surface->format, 255, 255, 255) :
        SDL_MapRGB(surface->format, 0, 0, 0);

    SDL_FillRect(surface, &rect, color);
}

void draw_environment(SDL_Surface *surface, struct Cell world[NUM_CELL]) {
    for (int i = 0; i < NUM_CELL; i++) {
        draw_cell(surface, world[i], world[i].live);
    }
}

void check_cell(struct Cell world[NUM_CELL]) {
    struct Cell world_copy[NUM_CELL];
    memcpy(world_copy, world, sizeof(world_copy));

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            struct Cell cell_to_check = world_copy[j + COLUMNS * i];
            int live = 0;

            if (i > 0 && j > 0)
                live += world_copy[(j - 1) + COLUMNS * (i - 1)].live;
            if (i > 0)
                live += world_copy[j + COLUMNS * (i - 1)].live;
            if (i > 0 && j < COLUMNS - 1)
                live += world_copy[(j + 1) + COLUMNS * (i - 1)].live;
            if (j > 0)
                live += world_copy[(j - 1) + COLUMNS * i].live;
            if (j < COLUMNS - 1)
                live += world_copy[(j + 1) + COLUMNS * i].live;
            if (j > 0 && i < ROWS - 1)
                live += world_copy[(j - 1) + COLUMNS * (i + 1)].live;
            if (i < ROWS - 1)
                live += world_copy[j + COLUMNS * (i + 1)].live;
            if (j < COLUMNS - 1 && i < ROWS - 1)
                live += world_copy[(j + 1) + COLUMNS * (i + 1)].live;

            if (cell_to_check.live == 1 && (live < 2 || live > 3))
                world[j + COLUMNS * i].live = 0;
            else if (cell_to_check.live == 0 && live == 3)
                world[j + COLUMNS * i].live = 1;
            else
                world[j + COLUMNS * i].live = cell_to_check.live;
        }
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Game of Life",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    if (!surface) {
        fprintf(stderr, "SDL_GetWindowSurface error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    int paused = 1;
    SDL_Event event;
    Uint32 color_gray = SDL_MapRGB(surface->format, 130, 130, 130);

    srand((unsigned int)time(NULL));

    struct Cell world[NUM_CELL];
    initialize_environment(world);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                    paused = !paused;
                    break;
                case SDLK_n:
                    if (paused)
                        check_cell(world);
                    break;
                case SDLK_r:
                    initialize_environment(world);
                    break;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x / CELL_SIZE;
                int y = event.button.y / CELL_SIZE;
                struct Cell cell = {x, y, 1};
                world[x + COLUMNS * y] = cell;
                draw_cell(surface, cell, 1);
            }
        }

        if (SDL_MUSTLOCK(surface))
            SDL_LockSurface(surface);

        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

        for (int i = 0; i < NUM_CELL; ++i)
            if (world[i].live)
                draw_cell(surface, world[i], 1);

        draw_grid(surface, color_gray);

        if (SDL_MUSTLOCK(surface))
            SDL_UnlockSurface(surface);

        SDL_UpdateWindowSurface(window);

        if (!paused)
            check_cell(world);

        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}