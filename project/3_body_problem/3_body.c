#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WIDTH   1200
#define HEIGHT  800

#define NUM_BODIES 3
#define TRAIL_BUF  5000
#define MIN_DIST   1.5

#define G        10000.0
#define EPSILON  1e-6

#define COL_BLACK      0x00000000
#define COL_YELLOW     0x00ffff00
#define COL_LIGHTBLUE  0x00007fff
#define COL_WHITE      0x00ffffff

typedef struct {
    double x, y;
    double vx, vy;
    double mass;
    double r;
} Planet;

typedef struct {
    int x[TRAIL_BUF];
    int y[TRAIL_BUF];
    int head;
    int size;
} Trail;

static void fill_circle(SDL_Surface *surf, int cx, int cy, int rad, Uint32 col)
{
    int rad2 = rad * rad;
    for (int dy = -rad; dy <= rad; ++dy) {
        for (int dx = -rad; dx <= rad; ++dx) {
            if (dx*dx + dy*dy <= rad2) {
                int px = cx + dx;
                int py = cy + dy;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    SDL_Rect pixel = { px, py, 1, 1 };
                    SDL_FillRect(surf, &pixel, col);
                }
            }
        }
    }
}

static void trail_push(Trail *t, int x, int y)
{
    if (t->size == 0) {
        t->x[0] = x;
        t->y[0] = y;
        t->head = 1 % TRAIL_BUF;
        t->size = 1;
        return;
    }

    int last = (t->head - 1 + TRAIL_BUF) % TRAIL_BUF;
    if (abs(x - t->x[last]) >= MIN_DIST || abs(y - t->y[last]) >= MIN_DIST) {
        t->x[t->head] = x;
        t->y[t->head] = y;
        t->head = (t->head + 1) % TRAIL_BUF;
        if (t->size < TRAIL_BUF) t->size++;
    }
}

static void trail_draw(SDL_Surface *surf, const Trail *t, Uint32 col)
{
    for (int i = 0; i < t->size; ++i) {
        int idx = (t->head - 1 - i + TRAIL_BUF) % TRAIL_BUF;
        SDL_Rect p = { t->x[idx], t->y[idx], 2, 2 };
        SDL_FillRect(surf, &p, col);
    }
}

static void accelerations(const Planet b[], double ax[], double ay[])
{
    for (int i = 0; i < NUM_BODIES; ++i) ax[i] = ay[i] = 0.0;

    for (int i = 0; i < NUM_BODIES; ++i) {
        for (int j = i + 1; j < NUM_BODIES; ++j) {
            double dx = b[j].x - b[i].x;
            double dy = b[j].y - b[i].y;
            double dist2 = dx*dx + dy*dy + EPSILON;
            double dist  = sqrt(dist2);

            double F  = (G * b[i].mass * b[j].mass) / dist2;
            double fx = F * dx / dist;
            double fy = F * dy / dist;

            ax[i] +=  fx / b[i].mass;
            ay[i] +=  fy / b[i].mass;
            ax[j] -=  fx / b[j].mass;
            ay[j] -=  fy / b[j].mass;
        }
    }
}

static void step_leapfrog(Planet b[], double dt)
{
    static double ax[NUM_BODIES], ay[NUM_BODIES];
    static int first = 1;

    if (first) {
        accelerations(b, ax, ay);
        first = 0;
    }

    for (int i = 0; i < NUM_BODIES; ++i) {
        b[i].vx += 0.5 * ax[i] * dt;
        b[i].vy += 0.5 * ay[i] * dt;
        b[i].x  +=      b[i].vx * dt;
        b[i].y  +=      b[i].vy * dt;
    }

    accelerations(b, ax, ay);

    for (int i = 0; i < NUM_BODIES; ++i) {
        b[i].vx += 0.5 * ax[i] * dt;
        b[i].vy += 0.5 * ay[i] * dt;
    }
}

static void recenter(Planet b[])
{
    double cx = 0, cy = 0, M = 0;
    for (int i = 0; i < NUM_BODIES; ++i) {
        cx += b[i].x * b[i].mass;
        cy += b[i].y * b[i].mass;
        M  += b[i].mass;
    }
    cx /= M;
    cy /= M;

    double dx = WIDTH / 2.0 - cx;
    double dy = HEIGHT / 2.0 - cy;
    for (int i = 0; i < NUM_BODIES; ++i) {
        b[i].x += dx;
        b[i].y += dy;
    }
}

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Three-Body Problem",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Surface *surf = SDL_GetWindowSurface(win);
    if (!surf) {
        fprintf(stderr, "SDL_GetWindowSurface: %s\n", SDL_GetError());
        return 1;
    }

    Planet bodies[NUM_BODIES];
    const double S  = 140.0;
    const double VS = 140.0;
    const double m  = 200.0;
    double cx = WIDTH / 2.0;
    double cy = HEIGHT / 2.0;

    bodies[0] = (Planet){ cx + (-0.97000436)*S, cy + ( 0.24308753)*S,
                          0.466203685*VS,  0.43236573*VS,   m, 15 };
    bodies[1] = (Planet){ cx,                 cy,
                         -0.93240737*VS, -0.86473146*VS,   m, 15 };
    bodies[2] = (Planet){ cx + ( 0.97000436)*S, cy + (-0.24308753)*S,
                          0.466203685*VS,  0.43236573*VS,   m, 15 };

    Trail trails[NUM_BODIES] = {0};

    int running = 1;
    SDL_Event ev;
    const double FIXED_DT = 0.0002;
    double accumulator = 0.0;
    Uint32 prev = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&ev))
            if (ev.type == SDL_QUIT) running = 0;

        Uint32 now = SDL_GetTicks();
        double frame_dt = (now - prev) / 1000.0;
        prev = now;
        if (frame_dt > 0.05) frame_dt = 0.05;
        accumulator += frame_dt;

        while (accumulator >= FIXED_DT) {
            step_leapfrog(bodies, FIXED_DT);
            accumulator -= FIXED_DT;
        }

        recenter(bodies);

        for (int i = 0; i < NUM_BODIES; ++i)
            trail_push(&trails[i], (int)bodies[i].x, (int)bodies[i].y);

        SDL_FillRect(surf, NULL, COL_BLACK);

        trail_draw(surf, &trails[0], COL_YELLOW);
        trail_draw(surf, &trails[1], COL_LIGHTBLUE);
        trail_draw(surf, &trails[2], COL_WHITE);

        fill_circle(surf, (int)bodies[0].x, (int)bodies[0].y, (int)bodies[0].r, COL_YELLOW);
        fill_circle(surf, (int)bodies[1].x, (int)bodies[1].y, (int)bodies[1].r, COL_LIGHTBLUE);
        fill_circle(surf, (int)bodies[2].x, (int)bodies[2].y, (int)bodies[2].r, COL_WHITE);

        SDL_UpdateWindowSurface(win);
        SDL_Delay(16);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}