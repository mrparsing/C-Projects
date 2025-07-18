# Rebuilding the Program from Scratch — Step by Step

## 🎬 Demo

![Demo](../assets/raytracing.gif)

## 0. What We Want to Build

* Show two circles on screen (one follows the mouse, the other stays put).
* Cast 200 rays from the center of the moving circle.
* “Trim” each ray when it hits the static circle to reveal its silhouette.
* Run it all at \~60 fps in a 900×600 SDL2 window.

---

## 1. Setting Up the Environment

### 1.1 Install SDL2

| Platform                        | Steps                                                                                           |
| ------------------------------- | ----------------------------------------------------------------------------------------------- |
| **Linux (Ubuntu/Debian)**       | `sudo apt update && sudo apt install libsdl2-dev`                                               |
| **Windows (MSYS2 + MinGW-w64)** | Install MSYS2 → open `mingw64` shell:<br>`pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2` |
| **macOS**                       | `brew install sdl2` (Homebrew handles everything)                                               |




## 2. File Skeleton

```c
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>

#define WIDTH  900
#define HEIGHT 600
#define RAYS_NUMBER 200

#define COLOR_BLACK 0xFF000000  // ARGB
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_GRAY  0xEFEFEFEF
```

---

## 3. Data Models

```c
struct Circle {
    double x, y, r;
};

struct Ray {
    double x_start, y_start;
    double x_end,   y_end;
    double angle;
};
```

* `double` makes trigonometry easier.
* `Ray` stores its angle in case we need it again.

---

## 4. DIY Rendering Routines

### 4.1 Draw Filled Circle

### What does `FillCircle` do?

This C function uses SDL to draw a filled white circle onto a surface. Here's a breakdown of how it works:

1. Compute the squared radius
```c
double rr = c.r * c.r;
```

This avoids computing a square root in the distance check by comparing squared distances.


2. Iterate over the bounding box of the circle
```c
for (int x = c.x - c.r; x <= c.x + c.r; x++)
    for (int y = c.y - c.r; y <= c.y + c.r; y++)
```

This loop scans through all the pixels in the square area that contains the circle.


3. Check if each pixel is inside the circle
```c
if ((x - c.x)*(x - c.x) + (y - c.y)*(y - c.y) <= rr)
```
This is the classic equation of a circle:
(x − center_x)² + (y − center_y)² ≤ radius²
If true, the point lies within the circle.

4. Set the pixel to white
```c
((Uint32*)surf->pixels)[y * surf->w + x] = COLOR_WHITE;
```

This line casts the pixels pointer to Uint32*, calculates the offset (row * width + column), and assigns the white color.


### 4.2 Bresenham Line Drawing

Why [Bresenham](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm)? It uses only integers — great for tight loops.

```c
void DrawLine(SDL_Surface *surf, int x0,int y0,int x1,int y1, Uint32 col) {
d    int dx = abs(x1-x0), dy = abs(y1-y0);
    int sx = (x0<x1?1:-1), sy = (y0<y1?1:-1);
    int err = dx - dy;
    while (1) {
        if (x0>=0 && x0<surf->w && y0>=0 && y0<surf->h)
            ((Uint32*)surf->pixels)[y0*surf->w + x0] = col;
        if (x0==x1 && y0==y1) break;
        int e2 = err<<1;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}
```

---

## 5. Ray Generation

The `generate_rays` function creates an array of rays originating from the center of a circle and spreading outward in all directions. Each ray has a fixed length and a unique angle.


```c
void generate_rays(struct Circle c, struct Ray r[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; ++i) {
        double ang = (double)i / RAYS_NUMBER * 2 * M_PI;
        r[i].x_start = c.x;
        r[i].y_start = c.y;
        r[i].x_end   = c.x + cos(ang) * 1000;
        r[i].y_end   = c.y + sin(ang) * 1000;
        r[i].angle   = ang;
    }
}
```
This function generates RAYS_NUMBER rays starting from the center of the circle c and pointing in evenly distributed directions around a full circle (360 degrees or (2\pi) radians).

For each ray:
1. Compute the angle
```c
double ang = (double)i / RAYS_NUMBER * 2 * M_PI;
```
- i is the ray index.
- Dividing i by RAYS_NUMBER gives a fraction between 0 and 1.
- Multiplying by 2π gives the angle in radians.
- This evenly spaces the rays around a full circle.

2. Set the ray’s starting point
```c
r[i].x_start = c.x;
r[i].y_start = c.y;
```
3. Set the ray’s ending point
```c
r[i].x_end = c.x + cos(ang) * 1000;
r[i].y_end = c.y + sin(ang) * 1000;
```
- Uses the angle to compute the direction of the ray using cosine and sine.
- The ray is extended from the center in that direction by a fixed length of 1000 units.
- All rays are the same length.

---

## 6. Ray-Circle Intersection

Math idea:

1. Parametrize the ray: `P(t) = P0 + t·d`, with `t ∈ [0,1]` for finite segments.
2. Enforce: `|P(t) − C|² = r²`
3. Solve the resulting quadratic equation.
4. If Δ < 0 → no intersection. Otherwise pick smallest valid `t` and trim the ray.

```c
void check_collision(struct Circle c, struct Ray r[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; ++i) {
        double dx = r[i].x_end - r[i].x_start;
        double dy = r[i].y_end - r[i].y_start;
        double fx = r[i].x_start - c.x;
        double fy = r[i].y_start - c.y;

        double a = dx*dx + dy*dy;
        double b = 2 * (fx*dx + fy*dy);
        double cc = fx*fx + fy*fy - c.r*c.r;

        double disc = b*b - 4*a*cc;
        if (disc < 0) continue;

        disc = sqrt(disc);
        double t1 = (-b - disc) / (2*a);
        double t2 = (-b + disc) / (2*a);

        double t = (t1>=0 && t1<=1) ? t1 :
                   (t2>=0 && t2<=1) ? t2 : -1;
        if (t > 0) {
            r[i].x_end = r[i].x_start + t*dx;
            r[i].y_end = r[i].y_start + t*dy;
        }
    }
}
```

---

## 7. Draw the Rays

```c
void FillRays(SDL_Surface *surf, struct Ray r[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; ++i)
        DrawLine(surf, r[i].x_start, r[i].y_start,
                        r[i].x_end,   r[i].y_end, COLOR_GRAY);
}
```

---

## 8. SDL Game Loop

```c
int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { /* error */ }

    SDL_Window  *win  = SDL_CreateWindow("Raytracing",
                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                          WIDTH, HEIGHT, 0);
    SDL_Surface *surf = SDL_GetWindowSurface(win);

    struct Circle player = {200,200, 50};
    struct Circle shadow = {600,300,110};
    struct Ray rays[RAYS_NUMBER];
    SDL_Rect screen = {0,0, WIDTH,HEIGHT};

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_MOUSEMOTION && (e.motion.state & SDL_BUTTON_LMASK)) {
                player.x = e.motion.x;
                player.y = e.motion.y;
            }
        }

        SDL_FillRect(surf, &screen, COLOR_BLACK);
        FillCircle(surf, player);
        FillCircle(surf, shadow);
        generate_rays(player, rays);
        check_collision(shadow, rays);
        FillRays(surf, rays);

        SDL_UpdateWindowSurface(win);
        SDL_Delay(10); // ~100 fps max
    }
    SDL_DestroyWindow(win);
    SDL_Quit();
}
```

---

## 9. Final Compilation

```bash
gcc main.c $(sdl2-config --cflags --libs) -lm -o raytracer
```

* `-lm` links the math library (cos, sin, sqrt).
* On Windows: add `-mwindows -lmingw32 -lSDL2main`.