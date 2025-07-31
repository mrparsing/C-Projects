#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <limits.h>

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 20
#define NUM_CELL (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)
#define ROWS HEIGHT / CELL_SIZE
#define COLUMNS WIDTH / CELL_SIZE

#define INF INT_MAX

int get_index(int x, int y)
{
    return y * COLUMNS + x;
}

int in_bounds(int x, int y)
{
    return x >= 0 && y >= 0 && x < COLUMNS && y < ROWS;
}

struct Cell
{
    int x;
    int y;
    int type; // 0 = vuoto, 1 = sorgente, 2 = obiettivo, 3 = muro, 4 = percorso, 5 = open set, 6 = closed set
};

void draw_grid(SDL_Surface *surface, Uint32 color)
{
    SDL_Rect line;

    for (int x = 0; x <= WIDTH; x += CELL_SIZE)
    {
        line.x = x;
        line.y = 0;
        line.w = 1;
        line.h = HEIGHT;
        SDL_FillRect(surface, &line, color);
    }

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

void draw_environment(SDL_Surface *surface, struct Cell environment[NUM_CELL], SDL_PixelFormat *format)
{
    for (int i = 0; i < NUM_CELL; i++)
    {
        Uint32 color;
        switch (environment[i].type)
        {
        case 1:
            color = SDL_MapRGB(format, 255, 255, 0);
            break; // source
        case 2:
            color = SDL_MapRGB(format, 0, 0, 255);
            break; // goal
        case 3:
            color = SDL_MapRGB(format, 128, 128, 128);
            break; // wall
        case 4:
            color = SDL_MapRGB(format, 255, 255, 255);
            break; // path
        case 5:
            color = SDL_MapRGB(format, 0, 255, 0);
            break; // open set
        case 6:
            color = SDL_MapRGB(format, 255, 0, 0);
            break; // closed set
        default:
            color = SDL_MapRGB(format, 0, 0, 0);
            break; // empty
        }
        draw_cell(surface, environment[i], color);
    }
}

int manhattan(struct Cell a, struct Cell b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

void a_star(struct Cell world[NUM_CELL], int source_index, int goal_index, SDL_Surface *surface, SDL_Window *window)
{
    int came_from[NUM_CELL];
    int g_score[NUM_CELL];
    int f_score[NUM_CELL];
    bool open_set[NUM_CELL] = {0};
    bool closed_set[NUM_CELL] = {0};

    for (int i = 0; i < NUM_CELL; i++)
    {
        g_score[i] = INF;
        f_score[i] = INF;
        came_from[i] = -1;
    }

    g_score[source_index] = 0;
    f_score[source_index] = manhattan(world[source_index], world[goal_index]);
    open_set[source_index] = true;

    while (1)
    {
        int current = -1;
        int min_f = INF;
        for (int i = 0; i < NUM_CELL; i++)
        {
            if (open_set[i] && f_score[i] < min_f)
            {
                min_f = f_score[i];
                current = i;
            }
        }

        if (current == -1)
            return;
        if (current == goal_index)
            break;

        open_set[current] = false;
        closed_set[current] = true;
        if (world[current].type != 1 && world[current].type != 2)
            world[current].type = 6; // closed set

        int x = world[current].x;
        int y = world[current].y;

        int dx[] = {1, -1, 0, 0};
        int dy[] = {0, 0, 1, -1};

        for (int dir = 0; dir < 4; dir++)
        {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (!in_bounds(nx, ny))
                continue;

            int neighbor = get_index(nx, ny);
            if (world[neighbor].type == 3 || closed_set[neighbor])
                continue;

            int tentative_g = g_score[current] + 1;
            if (!open_set[neighbor] || tentative_g < g_score[neighbor])
            {
                came_from[neighbor] = current;
                g_score[neighbor] = tentative_g;
                f_score[neighbor] = tentative_g + manhattan(world[neighbor], world[goal_index]);
                if (!open_set[neighbor])
                {
                    open_set[neighbor] = true;
                    if (world[neighbor].type != 2)
                        world[neighbor].type = 5; // open set
                }
            }
        }

        draw_environment(surface, world, surface->format);
        draw_grid(surface, SDL_MapRGB(surface->format, 50, 50, 50));
        SDL_UpdateWindowSurface(window);
        SDL_Delay(5);
    }

    int current = goal_index;
    while (came_from[current] != -1 && current != source_index)
    {
        if (world[current].type != 2 && world[current].type != 1)
            world[current].type = 4; // path
        current = came_from[current];

        draw_environment(surface, world, surface->format);
        draw_grid(surface, SDL_MapRGB(surface->format, 50, 50, 50));
        SDL_UpdateWindowSurface(window);
        SDL_Delay(20);
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("A* Visualization",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_Event event;

    int running = 1;
    int cell_type = 0;
    int source_index = -1;
    int goal_index = -1;
    int mouse_down = 0;

    struct Cell world[NUM_CELL];
    initialize_environment(world);

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;

            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_s:
                    cell_type = 1;
                    break;
                case SDLK_g:
                    cell_type = 2;
                    break;
                case SDLK_w:
                    cell_type = 3;
                    break;
                case SDLK_r:
                    for (int i = 0; i < NUM_CELL; i++)
                        world[i].type = 0;
                    break;
                case SDLK_SPACE:
                    if (source_index != -1 && goal_index != -1)
                        a_star(world, source_index, goal_index, surface, window);
                    break;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    mouse_down = 1;

                int x = event.button.x / CELL_SIZE;
                int y = event.button.y / CELL_SIZE;
                int index = get_index(x, y);

                if (index >= 0 && index < NUM_CELL)
                {
                    if (cell_type == 1)
                    {
                        if (source_index != -1)
                            world[source_index].type = 0;
                        source_index = index;
                    }
                    else if (cell_type == 2)
                    {
                        if (goal_index != -1)
                            world[goal_index].type = 0;
                        goal_index = index;
                    }
                    world[index].type = cell_type;
                }
            }

            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    mouse_down = 0;
            }

            else if (event.type == SDL_MOUSEMOTION && mouse_down)
            {
                int x = event.motion.x / CELL_SIZE;
                int y = event.motion.y / CELL_SIZE;
                int index = get_index(x, y);

                if (index >= 0 && index < NUM_CELL)
                {
                    // Evita di sovrascrivere sorgente/obiettivo
                    if (cell_type == 3 && world[index].type != 1 && world[index].type != 2)
                        world[index].type = 3; // muro
                }
            }
        }

        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
        draw_environment(surface, world, surface->format);
        draw_grid(surface, SDL_MapRGB(surface->format, 50, 50, 50));
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}