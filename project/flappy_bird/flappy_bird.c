#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>

#define WIDTH 900
#define HEIGHT 600
#define NUM_COLUMNS 10
#define GRAVITY 800.0

int score = 0;

typedef struct
{
    double x;
    double gap_y;
    double gap_height;
    double width;
} Column;

typedef struct
{
    double x;
    double y;
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
}

void jump(Bird *bird)
{
    bird->vy = -350.0;
}

void move_columns(Column *columns, int passed[NUM_COLUMNS])
{
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

            passed[i] = 0;
        }
    }
}

int check_collision(Column columns[NUM_COLUMNS], Bird bird)
{
    int bird_radius = 10;

    if (bird.y >= HEIGHT || bird.y <= 0)
        return 1;

    for (int i = 0; i < NUM_COLUMNS; i++)
    {
        if (bird.x + bird_radius > columns[i].x &&
            bird.x - bird_radius < columns[i].x + columns[i].width)
        {
            if (bird.y - bird_radius < columns[i].gap_y ||
                bird.y + bird_radius > columns[i].gap_y + columns[i].gap_height)
            {
                return 1;
            }
        }
    }

    return 0;
}

void update_score(SDL_Renderer *renderer, TTF_Font *font, int score)
{
    char text[32];
    snprintf(text, sizeof(text), "%d", score);

    SDL_Color color = {0, 0, 0, 0}; // black

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface)
    {
        printf("Text render error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture)
    {
        printf("Texture error: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect dst = {WIDTH / 2, 50, 0, 0}; // posizione in alto a sinistra
    SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
}

int main()
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
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
    TTF_Font *font = TTF_OpenFont("FreeSansBold.otf", 32);
    if (!font)
    {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;
    int start = 0;

    Column columns[NUM_COLUMNS];
    int passed[NUM_COLUMNS] = {0};
    Bird bird = {100, HEIGHT / 2, 10.0};

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
                    start = 1;
                    jump(&bird);
                    break;
                }
            }
        }

        generate_gap_column(renderer, columns);
        render_bird(renderer, bird);
        if (start)
        {
            move_columns(columns, passed);
            apply_gravity(&bird, dt);

            for (int i = 0; i < NUM_COLUMNS; i++)
            {
                if (!passed[i] && columns[i].x + columns[i].width < bird.x)
                {
                    passed[i] = 1;
                    score++;
                }
            }
            update_score(renderer, font, score);
            if (check_collision(columns, bird))
                start = 0;
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}