#include <stdio.h>
#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define DISTANCE 5.0f

struct Cube {
    int x;
    int y;
};

void project(float x, float y, float z, float *x2D, float *y2D) {
    float scale = DISTANCE / (z + DISTANCE);
    float screen_factor = scale * ((WIDTH < HEIGHT ? WIDTH : HEIGHT) / 2.0f);
    *x2D = x * screen_factor + WIDTH / 2.0f;
    *y2D = y * screen_factor + HEIGHT / 2.0f;
}

void DrawLine(SDL_Surface *surface, int x0, int y0, int x1, int y1, Uint32 color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x0 >= 0 && x0 < surface->w && y0 >= 0 && y0 < surface->h) {
            Uint32 *pixels = (Uint32 *)surface->pixels;
            pixels[(y0 * surface->w) + x0] = color;
        }
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void rotation(float x, float y, float z,
              float *x_rot, float *y_rot, float *z_rot,
              float angleX, float angleY, float angleZ) {
    // Rotation around X axis
    float y1 = y * cos(angleX) - z * sin(angleX);
    float z1 = y * sin(angleX) + z * cos(angleX);
    float x1 = x;

    // Rotation around Y axis
    float x2 = x1 * cos(angleY) + z1 * sin(angleY);
    float z2 = -x1 * sin(angleY) + z1 * cos(angleY);
    float y2 = y1;

    // Rotation around Z axis
    *x_rot = x2 * cos(angleZ) - y2 * sin(angleZ);
    *y_rot = x2 * sin(angleZ) + y2 * cos(angleZ);
    *z_rot = z2;
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Cube rotation",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    int running = 1;
    SDL_Event e;

    int cube[8][3] = {
        {-1, -1, -1}, {1, -1, -1},
        {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1}, {1, -1, 1},
        {1, 1, 1}, {-1, 1, 1}
    };

    struct Cube cube2D[8];

    float angleX = 0;
    float angleY = 0;
    float angleZ = 0;

    float z_offset = 10.0f;

    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        SDL_FillRect(surface, NULL, 0x000000);

        for (int i = 0; i < 8; i++) {
            float x2D, y2D;
            float x_r, y_r, z_r;

            rotation(cube[i][0], cube[i][1], cube[i][2],
                     &x_r, &y_r, &z_r,
                     angleX, angleY, angleZ);

            project(x_r, y_r, z_r + z_offset, &x2D, &y2D);

            cube2D[i].x = (int)x2D;
            cube2D[i].y = (int)y2D;
        }

        for (int i = 0; i < 12; i++) {
            int a = edges[i][0];
            int b = edges[i][1];
            DrawLine(surface, cube2D[a].x, cube2D[a].y, cube2D[b].x, cube2D[b].y, COLOR_WHITE);
        }

        angleX += 0.01f;
        angleY += 0.02f;
        angleZ += 0.015f;

        SDL_UpdateWindowSurface(window);
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}