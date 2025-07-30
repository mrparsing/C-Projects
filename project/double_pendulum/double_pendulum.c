#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_BLACK 0xFF000000
#define TRAIL_LEN 4000

typedef struct
{
    int x, y;
} Point;

Point trail[TRAIL_LEN];
int trail_index = 0;

typedef struct
{
    double theta1, theta2;
    double omega1, omega2;
    double L1, L2;
    double m1, m2;
} DoublePendulum;

typedef struct
{
    double x, y, r;
} Circle;

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

void DrawLine(SDL_Surface *surface, int x0, int y0, int x1, int y1, Uint32 color)
{
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
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

void simulate(DoublePendulum *dp, double g, double dt)
{
    double m1 = dp->m1, m2 = dp->m2;
    double L1 = dp->L1, L2 = dp->L2;
    double t1 = dp->theta1, t2 = dp->theta2;
    double w1 = dp->omega1, w2 = dp->omega2;

    double delta = t1 - t2;
    double den1 = (m1 + m2) * L1 - m2 * L1 * cos(delta) * cos(delta);
    double den2 = (L2 / L1) * den1;

    double num1 = -g * (2 * m1 + m2) * sin(t1);
    num1 -= m2 * g * sin(t1 - 2 * t2);
    num1 -= 2 * sin(delta) * m2 * (w2 * w2 * L2 + w1 * w1 * L1 * cos(delta));

    double num2 = 2 * sin(delta) * (w1 * w1 * L1 * (m1 + m2) + g * (m1 + m2) * cos(t1) + w2 * w2 * L2 * m2 * cos(delta));

    double a1 = num1 / den1;
    double a2 = num2 / den2;

    dp->omega1 += a1 * dt;
    dp->omega2 += a2 * dt;
    dp->theta1 += dp->omega1 * dt;
    dp->theta2 += dp->omega2 * dt;
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Double Pendulum",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    Uint32 COLOR_WHITE = SDL_MapRGB(surface->format, 255, 255, 255);

    SDL_Rect erase = {0, 0, WIDTH, HEIGHT};
    SDL_Event e;
    int running = 1;

    Circle origin = {WIDTH / 2.0, HEIGHT / 4.0, 5.0};
    Circle bob1 = {0, 0, 8.0};
    Circle bob2 = {0, 0, 8.0};

    DoublePendulum dp = {
        .theta1 = M_PI / 2,
        .theta2 = M_PI / 2,
        .omega1 = 0,
        .omega2 = 0,
        .L1 = 150,
        .L2 = 150,
        .m1 = 10,
        .m2 = 10};

    Uint32 prev = SDL_GetTicks();

    double damping = 0.999;

    while (running)
    {
        Uint32 now = SDL_GetTicks();
        double dt = (now - prev) / 1000.0;
        if (dt > 0.05)
            dt = 0.05;
        prev = now;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        simulate(&dp, 500, dt);

        dp.omega1 *= damping;
        dp.omega2 *= damping;

        bob1.x = origin.x + dp.L1 * sin(dp.theta1);
        bob1.y = origin.y + dp.L1 * cos(dp.theta1);

        bob2.x = bob1.x + dp.L2 * sin(dp.theta2);
        bob2.y = bob1.y + dp.L2 * cos(dp.theta2);
        trail[trail_index].x = (int)bob2.x;
        trail[trail_index].y = (int)bob2.y;
        trail_index = (trail_index + 1) % TRAIL_LEN;

        SDL_FillRect(surface, &erase, COLOR_BLACK);
        int trail_size = 3;
        for (int i = 0; i < TRAIL_LEN; i++)
        {
            int idx = (trail_index + i) % TRAIL_LEN;
            int tx = trail[idx].x;
            int ty = trail[idx].y;
            int alpha = (255 * i) / TRAIL_LEN;
            Uint32 col = SDL_MapRGB(surface->format, alpha, alpha, alpha);

            for (int dy = -trail_size / 2; dy <= trail_size / 2; dy++)
            {
                int yy = ty + dy;
                if (yy < 0 || yy >= surface->h)
                    continue;
                Uint32 *row = (Uint32 *)((Uint8 *)surface->pixels + yy * surface->pitch);
                for (int dx = -trail_size / 2; dx <= trail_size / 2; dx++)
                {
                    int xx = tx + dx;
                    if (xx < 0 || xx >= surface->w)
                        continue;
                    row[xx] = col;
                }
            }
        }

        double speed = fabs(dp.omega2);
        if (speed > 10)
            speed = 10;
        int red = (int)(255.0 * (speed / 10.0));
        int green = 255 - red;
        Uint32 color_dynamic = SDL_MapRGB(surface->format, red, green, 255 - red);

        DrawLine(surface, (int)origin.x, (int)origin.y, (int)bob1.x, (int)bob1.y, color_dynamic);
        DrawLine(surface, (int)bob1.x, (int)bob1.y, (int)bob2.x, (int)bob2.y, color_dynamic);

        FillCircle(surface, origin, SDL_MapRGB(surface->format, 255, 255, 255));
        FillCircle(surface, bob1, color_dynamic);
        FillCircle(surface, bob2, color_dynamic);

        SDL_UpdateWindowSurface(window);
        SDL_Delay(16);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}