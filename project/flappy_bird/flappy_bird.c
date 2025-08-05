#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define WIDTH 900
#define HEIGHT 600
#define NUM_COLUMNS 10
#define GRAVITY 800.0

typedef struct
{
    int x;
    int gap_y;
    int gap_height;
    int width;
} Column;

typedef struct
{
    double x;
    double y;
    double vx;
    double vy;
} Bird;

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
    SDL_SetRenderDrawColor(renderer, 22, 204, 1, 0); // green

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
        int gap_height = RandRange(100, 160);
        columns[i].gap_y = gap_y;
        columns[i].x = WIDTH + i * 200;
        columns[i].gap_height = gap_height;
        columns[i].width = 40;
    }
}

void render_bird(SDL_Renderer *renderer, Bird bird)
{
    SDL_SetRenderDrawColor(renderer, 255, 237, 35, 0);
    int radius = 10;
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w;
            int dy = radius - h;
            if (dx * dx + dy * dy <= radius * radius)
            {
                SDL_RenderDrawPoint(renderer, bird.x + dx, bird.y + dy);
            }
        }
    }
}

void apply_gravity(Bird *bird, double dt)
{
    bird->vy += GRAVITY * dt;
    bird->y += bird->vy * dt;
    bird->vx += 0.0 * dt;
    bird->x += bird->vx * dt;
}

void jump(Bird *bird)
{
    bird->vy = -350.0;
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
    Bird bird = {100, HEIGHT / 2, 10.0, 0.0};

    init_columns(columns);

    Uint32 prev_ticks = SDL_GetTicks();
    while (running)
    {
        SDL_SetRenderDrawColor(renderer, 39, 232, 245, 0);
        SDL_RenderClear(renderer);

        Uint32 now = SDL_GetTicks();
        double dt = (now - prev_ticks) / 1000.0; // in seconds
        prev_ticks = now;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_SPACE:
                    jump(&bird);
                    break;
                }
            }
        }
        for (int i = 0; i < NUM_COLUMNS; i++)
        {
            columns[i].x -= 2;

            // printf("%d\n", columns[9].x + columns[9].width);
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
        render_bird(renderer, bird);
        apply_gravity(&bird, dt);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}