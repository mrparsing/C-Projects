#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>


#define WIDTH 900
#define HEIGHT 600
#define GRAVITY 800.0 /* pixel per second^2*/
#define COLOR_BLACK 0xFF000000
#define REST_SIDE 0.7
#define REST_BALL 0.6 /* circle-to-circle rebound coefficient */

/* -------------------- Circle structure -------------------- */
typedef struct
{
    double x;
    double y;
    double r;
    double vy; // vertical speed in pixels/sec
    double vx;
} Circle;

/* -------------------- Dynamic array of circles -------------------- */
typedef struct
{
    Circle *array;
    size_t used;
    size_t size;
} CircleArray;

typedef struct
{
    double x;
    double y;
    double r;
} Quad;

typedef struct
{
    Quad *array;
    size_t used;
    size_t size;
} QuadArray;

void initCircleArray(CircleArray *a, size_t initialSize)
{
    a->array = (Circle *)malloc(initialSize * sizeof(Circle));
    a->used = 0;
    a->size = initialSize;
}

void initQuadArray(QuadArray *a, size_t initialSize)
{
    a->array = (Quad *)malloc(initialSize * sizeof(Quad));
    a->used = 0;
    a->size = initialSize;
}

void insertCircle(CircleArray *a, Circle c)
{
    if (a->used == a->size)
    {
        a->size *= 2;
        Circle *tmp = (Circle *)realloc(a->array, a->size * sizeof(Circle));
        if (!tmp)
        {
            fprintf(stderr, "realloc failed\n");
            return;
        }
        a->array = tmp;
    }
    a->array[a->used++] = c;
}

void insertQuad(QuadArray *a, Quad c)
{
    if (a->used == a->size)
    {
        a->size *= 2;
        Quad *tmp = (Quad *)realloc(a->array, a->size * sizeof(Quad));
        if (!tmp)
        {
            fprintf(stderr, "realloc failed\n");
            return;
        }
        a->array = tmp;
    }
    a->array[a->used++] = c;
}

void freeCircleArray(CircleArray *a)
{
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

void freeQuadArray(QuadArray *a)
{
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

/* -------------------- Filled circle drawing -------------------- */
static void FillCircle(SDL_Surface *surface, Circle circle, Uint32 color)
{
    /* Integer limits of the bounding box of the circle */
    int minx = (int)(circle.x - circle.r);
    int maxx = (int)(circle.x + circle.r);
    int miny = (int)(circle.y - circle.r);
    int maxy = (int)(circle.y + circle.r);

    double r2 = circle.r * circle.r;

    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);

    Uint8 *pixels = (Uint8 *)surface->pixels;
    int pitch = surface->pitch;               /* bytes per row */
    int bpp = surface->format->BytesPerPixel;

    for (int y = miny; y <= maxy; ++y)
    {
        for (int x = minx; x <= maxx; ++x)
        {
            /* Check bounds */
            if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
                continue;
            /* Distance from center */
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

static void FillQuad(SDL_Surface *surface, Quad quad, Uint32 color)
{
    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);
    SDL_Rect r;
    r.x = (int)(quad.x - quad.r);
    r.y = (int)(quad.y - quad.r);
    r.w = (int)(quad.r * 2);
    r.h = (int)(quad.r * 2);

    SDL_FillRect(surface, &r, color);
    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
}

static void resolveCircleQuad(Circle *c, const Quad *q)
{
    double left = q->x - q->r;
    double right = q->x + q->r;
    double top = q->y - q->r;
    double bottom = q->y + q->r;

    /* closest point of the rectangle to the center of the circle */
    double closestX = c->x < left ? left : (c->x > right ? right : c->x);
    double closestY = c->y < top ? top : (c->y > bottom ? bottom : c->y);

    double dx = c->x - closestX;
    double dy = c->y - closestY;
    double dist2 = dx * dx + dy * dy;

    if (dist2 > c->r * c->r)
        return;

    double dist = sqrt(dist2);
    double nx, ny;

    if (dist > 1e-8)
    {
        /* normal from the surface to the center of the circle */
        nx = dx / dist;
        ny = dy / dist;

        double penetration = c->r - dist;
        c->x += nx * penetration;
        c->y += ny * penetration;
    }
    else
    {
        /* center inside rectangle (dist=0): push towards nearest side */
        double dl = fabs(c->x - left);
        double dr = fabs(right - c->x);
        double dt = fabs(c->y - top);
        double db = fabs(bottom - c->y);
        double m = fmin(fmin(dl, dr), fmin(dt, db));
        if (m == dl)
        {
            nx = -1;
            ny = 0;
            c->x = left - 1e-6 + c->r;
        }
        else if (m == dr)
        {
            nx = 1;
            ny = 0;
            c->x = right + 1e-6 - c->r;
        }
        else if (m == dt)
        {
            nx = 0;
            ny = -1;
            c->y = top - 1e-6 + c->r;
        }
        else
        {
            nx = 0;
            ny = 1;
            c->y = bottom + 1e-6 - c->r;
        }
    }

    /* rebound */
    double vn = c->vx * nx + c->vy * ny;
    if (vn < 0.0)
    {
        double bounce = -(1.0 + REST_SIDE) * vn;
        c->vx += nx * bounce;
        c->vy += ny * bounce;
    }
}

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Ball gravity simulation",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    Uint32 COLOR_WHITE = SDL_MapRGB(surface->format, 255, 255, 255);
    Uint32 COLOR_BLUE = SDL_MapRGB(surface->format, 0, 0, 255);

    CircleArray circles;
    initCircleArray(&circles, 16);

    QuadArray quad;
    initQuadArray(&quad, 16);

    int running = 1;
    int mouseDown = 0;
    int wall = 0;
    double spawn_ball_accum = 0.0;
    double spawn_wall_accum = 0.0;
    const double spawn_interval = 0.05; // Creates ~20 balls per second while holding down
    SDL_Event e;

    Uint32 prev_ticks = SDL_GetTicks(); // ms from SDL init
    while (running)
    {
        Uint32 now = SDL_GetTicks();
        double dt = (now - prev_ticks) / 1000.0; // in seconds
        prev_ticks = now;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
            {
                mouseDown = 1;
                Circle c = {e.button.x, e.button.y, 10.0, 0.0, 0.0};
                insertCircle(&circles, c);
                FillCircle(surface, c, COLOR_BLUE);
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
            {
                mouseDown = 0;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT)
            {
                wall = 1;
                Quad q = {e.button.x, e.button.y, 10.0};
                insertQuad(&quad, q);
                FillQuad(surface, q, COLOR_WHITE);
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT)
            {
                wall = 0;
            }

            if (mouseDown)
            {
                spawn_ball_accum += dt;
                while (spawn_ball_accum >= spawn_interval)
                {
                    spawn_ball_accum -= spawn_interval;
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    Circle c = {mx, my, 10.0, 0.0, 0.0};
                    insertCircle(&circles, c);
                }
            }
            else
            {
                spawn_ball_accum = 0.0;
            }

            if (wall)
            {
                spawn_wall_accum += dt;
                while (spawn_wall_accum >= spawn_interval)
                {
                    spawn_wall_accum -= spawn_interval;
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    Quad q = {mx, my, 10.0};
                    insertQuad(&quad, q);
                }
            }
            else
            {
                spawn_wall_accum = 0.0;
            }
        }
        for (size_t i = 0; i < circles.used; ++i)
        {
            circles.array[i].vy += GRAVITY * dt;            // accelerate downwards
            circles.array[i].y += circles.array[i].vy * dt; // update position

            circles.array[i].vx += 0.0 * dt;
            circles.array[i].x += circles.array[i].vx * dt;
        }

        SDL_FillRect(surface, NULL, COLOR_BLACK); // clear

        for (size_t i = 0; i < circles.used; ++i)
        {
            for (size_t q = 0; q < quad.used; ++q)
            {
                resolveCircleQuad(&circles.array[i], &quad.array[q]);
            }
        }
        for (size_t i = 0; i < quad.used; ++i)
        {
            FillQuad(surface, quad.array[i], COLOR_WHITE);
        }
        for (size_t i = 0; i < circles.used; ++i)
        {
            FillCircle(surface, circles.array[i], COLOR_BLUE);

            // FLOOR COLLISIONS
            if (circles.array[i].y + circles.array[i].r >= HEIGHT)
            {
                circles.array[i].y = HEIGHT - circles.array[i].r;
                circles.array[i].vy = -circles.array[i].vy * 0.7; // 70% energia
                if (fabs(circles.array[i].vy) < 5.0)
                    circles.array[i].vy = 0.0; // ferma se molto lento
            }

            // LEFT WALL
            if (circles.array[i].x - circles.array[i].r < 0)
            {
                circles.array[i].x = circles.array[i].r;
                circles.array[i].vx = -circles.array[i].vx * REST_SIDE;
            }

            // RIGHT WALL
            if (circles.array[i].x + circles.array[i].r >= WIDTH)
            {
                circles.array[i].x = WIDTH - circles.array[i].r;
                circles.array[i].vx = -circles.array[i].vx * REST_SIDE;
            }

            for (size_t k = 0; k < circles.used; ++k)
            {
                for (size_t j = k + 1; j < circles.used; ++j)
                {

                    double dx = circles.array[j].x - circles.array[k].x;
                    double dy = circles.array[j].y - circles.array[k].y;
                    double minDist = circles.array[k].r + circles.array[j].r;
                    double dist2 = dx * dx + dy * dy;

                    if (dist2 >= minDist * minDist)
                    {
                        continue;
                    }

                    double dist = sqrt(dist2);

                    /* normal contact */
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

                    double penetration = minDist - dist;
                    double half = penetration * 0.5;
                    circles.array[k].x -= nx * half;
                    circles.array[k].y -= ny * half;
                    circles.array[j].x += nx * half;
                    circles.array[j].y += ny * half;

                    double rvx = circles.array[j].vx - circles.array[k].vx;
                    double rvy = circles.array[j].vy - circles.array[k].vy;
                    double vn = rvx * nx + rvy * ny;

                    if (vn > 0.0)
                    {
                        continue;
                    }

                    double jimp = -(1.0 + REST_BALL) * vn / 2.0; // / (1/m1 + 1/m2) = /2

                    circles.array[k].vx -= jimp * nx;
                    circles.array[k].vy -= jimp * ny;
                    circles.array[j].vx += jimp * nx;
                    circles.array[j].vy += jimp * ny;
                }
            }
        }

        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    freeCircleArray(&circles);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
