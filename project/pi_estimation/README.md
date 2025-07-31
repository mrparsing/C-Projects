# Monte Carlo π Estimator

This program estimates **π** by randomly sampling points in a square and counting how many fall inside an inscribed circle. It also visualizes the process by plotting the circle outline and all sample points in an SDL window.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build
./pi_estimation
```

---

## 2. How It Works (Math Recap)

* We generate `NUM_POINTS` random points uniformly in a **square** of side `2R` (centered at the origin).

* The fraction that land **inside the circle** of radius `R` is approximately the circle’s area divided by the square’s area:

  $\frac{\#\text{inside}}{\#\text{total}} \approx \frac{\pi R^2}{(2R)^2} = \frac{\pi}{4}$

* So, $\pi \approx 4 \times \#\text{inside} / \#\text{total}$.

* The program prints the estimate to stdout.

---

## 3. Code Walkthrough

### 3.1 Constants & Structs

```c
#define WIDTH 900
#define HEIGHT 600
#define R 200
#define NUM_POINTS 100000
```

* `R` is the circle radius (in pixels).
* `NUM_POINTS` controls Monte Carlo accuracy (larger ⇒ better).

```c
struct Point { double x; double y; }; // only used for clarity in helpers
```

### 3.2 Drawing the Circle Border

```c
void generate_circle(SDL_Surface *surface) {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            double dx = i - WIDTH / 2;
            double dy = j - HEIGHT / 2;
            double distance = sqrt(dx * dx + dy * dy);
            if (distance >= R - 1 && distance <= R + 1) {
                SDL_Rect pixel = {i, j, 1, 1};
                SDL_FillRect(surface, &pixel, COLOR_WHITE);
            }
        }
    }
}
```

* Brute‑forces every pixel, marking a 2‑pixel‑thick annulus as the circle outline.

### 3.3 Plotting Points

```c
void print_point(SDL_Surface *surface, double dx, double dy) {
    SDL_Rect pixel = {(int)dx, (int)dy, 1, 1};
    SDL_FillRect(surface, &pixel, COLOR_WHITE);
}
```

* Single‑pixel "star" at the given screen coords.

### 3.4 Random Sampling & π Estimate

```c
void generate_random_point(SDL_Surface *surface) {
    int counter = 0;
    for (int i = 0; i < NUM_POINTS; i++) {
        double x = ((double)rand()/RAND_MAX)*2*R - R; // [-R, R]
        double y = ((double)rand()/RAND_MAX)*2*R - R;
        double distance = sqrt(x*x + y*y);
        if (distance <= R) counter++;
        double dx = x + WIDTH/2; // move to window coords
        double dy = y + HEIGHT/2;
        print_point(surface, dx, dy);
    }
    double pi_estimation = 4.0 * counter / NUM_POINTS;
    printf("PI estimation: %f\n", pi_estimation);
}
```

* Uses `rand()` (uniform on `[0,RAND_MAX]`) scaled to `[−R, R]`.
* Counts hits, draws each point, then prints the estimate.

### 3.5 Main Loop

```c
int main(void) {
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("PI Approximation", ...);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    generate_circle(surface);
    generate_random_point(surface);
    SDL_UpdateWindowSurface(window);

    // keep window open
    while (running) { SDL_PollEvent(...); SDL_Delay(10); }
}
```

* Seed RNG once.
* Draw once, then idle until the user closes the window.