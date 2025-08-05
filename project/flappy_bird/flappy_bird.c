#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#define WIDTH 600
#define HEIGHT 600
#define NUM_COLUMNS 10
#define GRAVITY 800.0

int score = 0;

typedef struct
{
    double x;
    double gap_y;
    double gap_height; // empty space
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

/* Find the rightmost pipe */
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

void render_columns(SDL_Renderer *renderer, SDL_Texture *pipe_texture, Column columns[NUM_COLUMNS])
{
    for (int i = 0; i < NUM_COLUMNS; i++)
    {
        int top_height = (int)columns[i].gap_y;
        int bottom_height = (int)(HEIGHT - (columns[i].gap_y + columns[i].gap_height));

        if (top_height > 0)
        {
            SDL_Rect top = {
                .x = (int)columns[i].x,
                .y = 0,
                .w = (int)columns[i].width,
                .h = top_height};
            SDL_RenderCopyEx(renderer, pipe_texture, NULL, &top, 0, NULL, SDL_FLIP_VERTICAL);
        }

        if (bottom_height > 0)
        {
            SDL_Rect bottom = {
                .x = (int)columns[i].x,
                .y = (int)(columns[i].gap_y + columns[i].gap_height),
                .w = (int)columns[i].width,
                .h = bottom_height};
            SDL_RenderCopy(renderer, pipe_texture, NULL, &bottom);
        }
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

void render_bird(SDL_Renderer *renderer, SDL_Texture *bird_texture, Bird bird)
{
    int bird_width = 32;
    int bird_height = 32;

    SDL_Rect dest = {
        (int)(bird.x - bird_width / 2),
        (int)(bird.y - bird_height / 2),
        bird_width,
        bird_height};

    SDL_RenderCopy(renderer, bird_texture, NULL, &dest);
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
            // I return the i-th column to the right
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

void render_text(SDL_Renderer *renderer, TTF_Font *font, int game_over)
{
    char text[64];
    if (game_over)
        snprintf(text, sizeof(text), "SCORE: %d  |  Press R to replay", score);
    else
        snprintf(text, sizeof(text), "%d", score);

    SDL_Color color = {0, 0, 0, 0}; // black

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
        return;

    SDL_Rect dst = {0, 50, 0, 0};
    SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
    dst.x = (WIDTH - dst.w) / 2;

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
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
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

    // FONT
    TTF_Font *font = TTF_OpenFont("FreeSansBold.otf", 32);
    if (!font)
    {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // TEXTURES
    SDL_Surface *bird_surface = IMG_Load("textures/bird.png");
    if (!bird_surface)
    {
        printf("IMG_Load Error: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Texture *bird_texture = SDL_CreateTextureFromSurface(renderer, bird_surface);
    SDL_FreeSurface(bird_surface);

    SDL_SetTextureBlendMode(bird_texture, SDL_BLENDMODE_BLEND);
    if (!bird_texture)
    {
        printf("Failed to load bird texture: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *bg_surface = IMG_Load("textures/background.png");
    if (!bg_surface)
    {
        printf("Failed to load background: %s\n", IMG_GetError());
    }
    SDL_Texture *bg_texture = SDL_CreateTextureFromSurface(renderer, bg_surface);
    SDL_FreeSurface(bg_surface);

    SDL_Surface *pipe_surface = IMG_Load("textures/pipe.png");
    if (!pipe_surface)
    {
        printf("Failed to load pipe: %s\n", IMG_GetError());
    }
    SDL_Texture *pipe_texture = SDL_CreateTextureFromSurface(renderer, pipe_surface);
    SDL_FreeSurface(pipe_surface);

    int running = 1;
    SDL_Event event;
    int playing = 0;
    int game_over;

    Column columns[NUM_COLUMNS];
    int passed[NUM_COLUMNS] = {0};
    Bird bird = {100, HEIGHT / 2, 10.0};

    init_columns(columns);

    Uint32 prev_ticks = SDL_GetTicks();
    while (running)
    {
        SDL_SetRenderDrawColor(renderer, 39, 232, 245, 0);
        SDL_RenderClear(renderer);

        SDL_Rect bg_dest = {0, 0, WIDTH, HEIGHT};
        SDL_RenderCopy(renderer, bg_texture, NULL, &bg_dest);

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
                    if (!game_over)
                    {
                        jump(&bird);
                        playing = 1;
                    }
                    break;
                case SDLK_r: // reset
                    bird.x = 100;
                    bird.y = HEIGHT / 2;
                    bird.vy = 10.0;
                    playing = 1;
                    game_over = 0;
                    score = 0;
                    init_columns(columns);
                    for (int i = 0; i < NUM_COLUMNS; i++)
                        passed[i] = 0;
                }
            }
        }

        render_columns(renderer, pipe_texture, columns);
        render_bird(renderer, bird_texture, bird);

        if (playing)
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
            render_text(renderer, font, 0);
            if (check_collision(columns, bird))
            {
                playing = 0;
                game_over = 1;
            }
        }
        render_text(renderer, font, game_over);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}