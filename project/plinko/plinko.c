#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 800
#define COLOR_WHITE 0xffffffff
#define GRAVITY 800.0
#define COLOR_BLACK 0xFF000000
#define REST_SIDE 0.7
#define REST_BALL 0.6
#define RADIUS 10.0
#define MAX_BALLS 200 // Maximum number of balls allowed

typedef struct
{
    double x, y, r, vy, vx;
    int snapped;
} Circle;

typedef struct
{
    double x, y, r;
} Obstacle;

typedef struct
{
    Circle *array;
    size_t used, size;
} CircleArray;

typedef struct
{
    Obstacle *array;
    size_t used, size;
} ObstacleArray;

void initArray(CircleArray *a, size_t initialSize)
{
    a->array = malloc(initialSize * sizeof(Circle));
    a->used = 0;
    a->size = initialSize;
}

void insertCircle(CircleArray *a, Circle c)
{
    if (a->used == a->size)
    {
        a->size = a->size ? a->size * 2 : 1;
        Circle *tmp = realloc(a->array, a->size * sizeof(Circle));
        if (!tmp)
            return;
        a->array = tmp;
    }
    a->array[a->used++] = c;
}

void initObstacles(ObstacleArray *a, size_t initialSize)
{
    a->array = malloc(initialSize * sizeof(Obstacle));
    a->used = 0;
    a->size = initialSize;
}

void insertObstacle(ObstacleArray *a, Obstacle o)
{
    if (a->used == a->size)
    {
        a->size = a->size ? a->size * 2 : 1;
        Obstacle *tmp = realloc(a->array, a->size * sizeof(Obstacle));
        if (!tmp)
            return;
        a->array = tmp;
    }
    a->array[a->used++] = o;
}

void FillCircle(SDL_Surface *surface, Circle circle, Uint32 color)
{
    int minx = (int)(circle.x - circle.r);
    int maxx = (int)(circle.x + circle.r);
    int miny = (int)(circle.y - circle.r);
    int maxy = (int)(circle.y + circle.r);
    double r2 = circle.r * circle.r;

    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);
    Uint8 *pixels = (Uint8 *)surface->pixels;
    int pitch = surface->pitch;
    int bpp = surface->format->BytesPerPixel;

    for (int y = miny; y <= maxy; ++y)
    {
        for (int x = minx; x <= maxx; ++x)
        {
            if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
                continue;
            double dx = x - circle.x;
            double dy = y - circle.y;
            if (dx * dx + dy * dy <= r2)
            {
                Uint8 *p = pixels + y * pitch + x * bpp;
                *(Uint32 *)p = color;
            }
        }
    }
    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
}

void apply_gravity(CircleArray *circles, double dt)
{
    for (size_t i = 0; i < circles->used; ++i)
    {
        if (circles->array[i].snapped)
            continue;

        circles->array[i].vy += GRAVITY * dt;
        circles->array[i].y += circles->array[i].vy * dt;
        circles->array[i].x += circles->array[i].vx * dt;
    }
}

// Resolve collisions with walls and floor
void resolve_walls_and_floor(CircleArray *circles)
{
    for (size_t i = 0; i < circles->used; ++i)
    {
        // FLOOR COLLISIONS
        if (circles->array[i].y + circles->array[i].r >= HEIGHT)
        {
            circles->array[i].y = HEIGHT - circles->array[i].r;
            if (circles->array[i].vy > 0)
            {                               // Only if falling
                circles->array[i].vy = 0.0; // Stop vertical velocity
            }
            // Apply horizontal friction
            circles->array[i].vx *= 0.9;
            // Stop if velocities are very low
            if (fabs(circles->array[i].vx) < 1.0 && fabs(circles->array[i].vy) < 1.0)
            {
                circles->array[i].vx = 0.0;
                circles->array[i].vy = 0.0;
                circles->array[i].snapped = 1;
            }
        }

        // LEFT WALL
        if (circles->array[i].x - circles->array[i].r < 0)
        {
            circles->array[i].x = circles->array[i].r;
            circles->array[i].vx = -circles->array[i].vx * REST_SIDE;
        }

        // RIGHT WALL
        if (circles->array[i].x + circles->array[i].r >= WIDTH)
        {
            circles->array[i].x = WIDTH - circles->array[i].r;
            circles->array[i].vx = -circles->array[i].vx * REST_SIDE;
        }
    }
}

// Resolve ball-ball collisions
void resolve_ball_ball_collisions(CircleArray *circles)
{
    for (size_t k = 0; k < circles->used; ++k)
    {
        for (size_t j = k + 1; j < circles->used; ++j)
        {
            double dx = circles->array[j].x - circles->array[k].x;
            double dy = circles->array[j].y - circles->array[k].y;
            double minDist = circles->array[k].r + circles->array[j].r;
            double dist2 = dx * dx + dy * dy;

            if (dist2 >= minDist * minDist)
                continue;

            double dist = sqrt(dist2);
            double nx, ny;
            if (dist > 0.0)
            {
                nx = dx / dist;
                ny = dy / dist;
            }
            else
            {
                nx = 1.0;
                ny = 0.0;
                dist = 0.0;
            }

            double penetration = fmin(minDist - dist + 0.001, 1.0); // max 1px push
            double half = penetration * 1.0;

            double mass_k = circles->array[k].snapped ? 100.0 : 1.0;
            double mass_j = circles->array[j].snapped ? 100.0 : 1.0;
            double total = mass_k + mass_j;
            circles->array[k].x -= nx * penetration * (mass_j / total);
            circles->array[k].y -= ny * penetration * (mass_j / total);
            circles->array[j].x += nx * penetration * (mass_k / total);
            circles->array[j].y += ny * penetration * (mass_k / total);

            double rvx = circles->array[j].vx - circles->array[k].vx;
            double rvy = circles->array[j].vy - circles->array[k].vy;
            double vn = rvx * nx + rvy * ny;

            if (vn > 0.0)
                continue;

            double jimp = -(1.0 + REST_BALL) * vn / 2.0;
            circles->array[k].vx -= jimp * nx;
            circles->array[k].vy -= jimp * ny;
            circles->array[j].vx += jimp * nx;
            circles->array[j].vy += jimp * ny;
        }
    }
}

void check_obstacle_collisions(CircleArray *circles, ObstacleArray *obstacles)
{
    for (size_t i = 0; i < circles->used; ++i)
    {
        for (size_t j = 0; j < obstacles->used; ++j)
        {
            double dx = circles->array[i].x - obstacles->array[j].x;
            double dy = circles->array[i].y - obstacles->array[j].y;
            double dist2 = dx * dx + dy * dy;
            double minDist = circles->array[i].r + obstacles->array[j].r;

            if (dist2 < minDist * minDist)
            {
                double dist = sqrt(dist2);
                double nx = dx / dist;
                double ny = dy / dist;

                double overlap = minDist - dist;
                circles->array[i].x += nx * overlap;
                circles->array[i].y += ny * overlap;

                double vn = circles->array[i].vx * nx + circles->array[i].vy * ny;
                if (vn < 0)
                {
                    double jimp = -(1.0 + REST_BALL) * vn;
                    circles->array[i].vx += jimp * nx;
                    circles->array[i].vy += jimp * ny;
                }
            }
        }
    }
}

void create_obstacles(ObstacleArray *obstacles)
{
    for (int j = 100; j < HEIGHT - 350; j += 40)
    {
        for (int i = 0; i < WIDTH; i += 40)
        {
            double x = i + (j / 40 % 2) * 20;
            double y = j;
            Obstacle o = {x, y, 6.0};
            insertObstacle(obstacles, o);
        }
    }
}

void draw_obstacles(SDL_Surface *surface, ObstacleArray *obstacles)
{
    for (size_t i = 0; i < obstacles->used; i++)
    {
        Circle c = {obstacles->array[i].x, obstacles->array[i].y, obstacles->array[i].r, 0, 0, 0};
        FillCircle(surface, c, COLOR_WHITE);
    }
}

void DrawLine(SDL_Surface *surface, int x0, int y0, int x1, int y1, Uint32 color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        if (x0 >= 0 && x0 < surface->w && y0 >= 0 && y0 < surface->h)
        {
            Uint32 *pixels = (Uint32 *)surface->pixels;
            pixels[(y0 * surface->w) + x0] = color;
        }
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_slot(SDL_Surface *surface)
{
    int slot_height = 320;
    for (int i = 0; i < WIDTH; i += 20)
    {
        DrawLine(surface, i, HEIGHT, i, HEIGHT - slot_height, COLOR_WHITE);
    }
}

// Ball-line collision detection - FIXED VERSION
int ballLineCollision(double cx, double cy, double r, double lineX, double y1, double y2)
{
    if (y1 > y2)
    {
        double tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    // Calculate distance to line
    double distX = fabs(cx - lineX);

    // If ball is too far horizontally, no collision
    if (distX > r)
        return 0;

    // Project ball center onto the line segment
    double projY = cy;
    if (projY < y1)
        projY = y1;
    if (projY > y2)
        projY = y2;

    // Calculate distance to projected point
    double dx = cx - lineX;
    double dy = cy - projY;
    double dist2 = dx * dx + dy * dy;

    // Check if distance is within radius
    return (dist2 <= r * r);
}

void check_line_collisions(CircleArray *circles)
{
    int slot_height = 300;
    for (size_t i = 0; i < circles->used; ++i)
    {
        for (int x = 0; x < WIDTH; x += 20)
        {
            if (ballLineCollision(circles->array[i].x, circles->array[i].y,
                                  circles->array[i].r, x, HEIGHT - slot_height, HEIGHT))
            {
                double overlap = circles->array[i].r - fabs(circles->array[i].x - x);
                if (overlap > 0)
                {
                    // Push ball away from the line
                    if (circles->array[i].x < x)
                    {
                        circles->array[i].x -= overlap * 0.5;
                    }
                    else
                    {
                        circles->array[i].x += overlap * 0.5;
                    }

                    // Only bounce if ball is moving toward the line
                    double vx = circles->array[i].vx;
                    if ((vx > 0 && circles->array[i].x < x) ||
                        (vx < 0 && circles->array[i].x > x))
                    {
                        circles->array[i].vx *= -REST_SIDE;
                    }
                }
            }
        }
    }
}

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Plinko", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    CircleArray circles;
    initArray(&circles, 100);
    ObstacleArray obstacles;
    initObstacles(&obstacles, 100);
    create_obstacles(&obstacles); // Create obstacles once

    int running = 1, mouseDown = 0;
    double spawn_ball_accum = 0.0;
    const double spawn_interval = 0.06;
    SDL_Event e;
    Uint32 prev_ticks = SDL_GetTicks();
    int auto_spawn_count = 200; // Numero di palline da spawnare
    double auto_spawn_timer = 0.0;
    const double auto_spawn_interval = 0.15;

    while (running)
    {
        Uint32 now = SDL_GetTicks();
        double dt = (now - prev_ticks) / 1000.0;
        if (dt > 0.05)
            dt = 0.05; // massimo 50 ms per tick
        prev_ticks = now;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
            {
                mouseDown = 1;
                Circle c = {e.button.x, e.button.y, RADIUS, 0.0, 0.0, 0};
                insertCircle(&circles, c);
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
            {
                mouseDown = 0;
            }
        }

        if (auto_spawn_count > 0)
        {
            auto_spawn_timer += dt;
            if (auto_spawn_timer >= auto_spawn_interval)
            {
                auto_spawn_timer = 0.0;

                // Crea una pallina al centro in alto con piccola variazione casuale
                double x = WIDTH / 2 + (rand() % 20 - 10); // -10 a +10 pixel
                Circle c = {x, -30, RADIUS, 0.0, 0.0, 0};
                insertCircle(&circles, c);

                auto_spawn_count--;
            }
        }

        if (mouseDown)
        {
            spawn_ball_accum += dt;
            while (spawn_ball_accum >= spawn_interval)
            {
                spawn_ball_accum -= spawn_interval;
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                Circle c = {mx, my, RADIUS, 0.0, 0.0, 0};
                insertCircle(&circles, c);
            }
        }
        else
        {
            spawn_ball_accum = 0.0;
        }

        apply_gravity(&circles, dt);
        SDL_FillRect(surface, NULL, COLOR_BLACK);

        // Collision resolution with multiple iterations
        for (int iter = 0; iter < circles.used / 2; iter++)
        {
            resolve_walls_and_floor(&circles);
            check_obstacle_collisions(&circles, &obstacles);
            check_line_collisions(&circles);
            resolve_ball_ball_collisions(&circles);
        }

        // Draw everything
        draw_obstacles(surface, &obstacles);
        draw_slot(surface);
        for (size_t i = 0; i < circles.used; i++)
        {
            FillCircle(surface, circles.array[i], COLOR_WHITE);
        }

        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }
    free(circles.array);
    free(obstacles.array);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}