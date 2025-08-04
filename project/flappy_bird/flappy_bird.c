#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define NUM_COLUMNS 10

typedef struct
{
    int x;
    int gap_y;
    int gap_height;
    int width;
} Column;

int RandRange(int Min, int Max)
{
    int diff = Max - Min;
    return (int)(((double)(diff + 1) / RAND_MAX) * rand() + Min);
}

int find_rightmost_x(Column columns[NUM_COLUMNS])
{
    int max_x = 0;
    for (int i = 0; i < NUM_COLUMNS; i++)
    {
        if (columns[i].x > max_x)
            max_x = columns[i].x;
    }
    return max_x;
}

void generate_gap_column(SDL_Renderer *renderer, Column columns[NUM_COLUMNS])
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green

    for (int i = 0; i < NUM_COLUMNS; i++)
    {
        SDL_Rect top = {
            .x = columns[i].x,
            .y = 0,
            .w = columns[i].width,
            .h = columns[i].gap_y};

        SDL_Rect bottom = {
            .x = columns[i].x,
            .y = columns[i].gap_y + columns[i].gap_height,
            .w = columns[i].width,
            .h = HEIGHT - (columns[i].gap_y + columns[i].gap_height)};

        SDL_RenderFillRect(renderer, &top);
        SDL_RenderFillRect(renderer, &bottom);
    }
}

void init_columns(Column columns[NUM_COLUMNS])
{
    for (int i = 0; i < NUM_COLUMNS; i++)
    {
        int gap_y = rand() % (HEIGHT - 200);
        int gap_height = RandRange(100, 200);
        columns[i].gap_y = gap_y;
        columns[i].x = WIDTH + i * 200;
        columns[i].gap_height = gap_height;
        columns[i].width = 40;
    }
}

int main()
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Flappy Bird",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;

    Column columns[10];

    init_columns(columns);

    while (running)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
        }

        for (int i = 0; i < NUM_COLUMNS; i++)
        {
            columns[i].x -= 2;

            //printf("%d\n", columns[9].x + columns[9].width);
            if (columns[i].x + columns[i].width < 0)
            {
                // printf("Rimetto a destra la colonna %d\n", i);
                int rightmost = find_rightmost_x(columns);
                columns[i].x = rightmost + 200;
                columns[i].gap_y = rand() % (HEIGHT - 200);
                columns[i].gap_height = 100 + rand() % 80;
            }
        }

        generate_gap_column(renderer, columns);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}