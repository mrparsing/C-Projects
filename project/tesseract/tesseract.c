#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600
#define SCALE 1000
#define DISTANCE_4D 3.0f // distance from viewer to 4D space
#define DISTANCE_3D 5.0f // distance from viewer to 3D space

typedef struct
{
    float x, y, z, w;
} Vec4;

typedef struct
{
    float x, y, z;
} Vec3;

typedef struct
{
    float x, y;
} Vec2;

// there are 6 possible rotation planes in 4D space

// Rotation in XY plane
void rotate_xy(Vec4 *v, float angle)
{
    float x = v->x, y = v->y;
    v->x = x * cosf(angle) - y * sinf(angle);
    v->y = x * sinf(angle) + y * cosf(angle);
}

// rotation in XZ plane
void rotate_xz(Vec4 *v, float angle)
{
    float x = v->x, z = v->z;
    v->x = x * cosf(angle) - z * sinf(angle);
    v->z = x * sinf(angle) + z * cosf(angle);
}

// Rotation in XW plane
void rotate_xw(Vec4 *v, float angle)
{
    float x = v->x, w = v->w;
    v->x = x * cosf(angle) - w * sinf(angle);
    v->w = x * sinf(angle) + w * cosf(angle);
}

// Rotation in YZ plane
void rotate_yz(Vec4 *v, float angle)
{
    float y = v->y, z = v->z;
    v->y = y * cosf(angle) - z * sinf(angle);
    v->z = y * sinf(angle) + z * cosf(angle);
}

// Rotation in YW plane
void rotate_yw(Vec4 *v, float angle)
{
    float y = v->y, w = v->w;
    v->y = y * cosf(angle) - w * sinf(angle);
    v->w = y * sinf(angle) + w * cosf(angle);
}

// rotation in ZW plane
void rotate_zw(Vec4 *v, float angle)
{
    float z = v->z, w = v->w;
    v->z = z * cosf(angle) - w * sinf(angle);
    v->w = z * sinf(angle) + w * cosf(angle);
}

// project 4D point to 3D using perspective projection
Vec3 project_4D_to_3D(Vec4 v)
{
    float denom = DISTANCE_4D - v.w;
    if (fabs(denom) < 0.001f)
        denom = 0.001f; // to avoid division by zero
    float w_factor = 1.0f / denom;
    Vec3 result = {v.x * w_factor, v.y * w_factor, v.z * w_factor};
    return result;
}

// project 3D point to 2D screen coordinates
Vec2 project_3D_to_2D(Vec3 v)
{
    float denom = DISTANCE_3D - v.z;
    if (fabs(denom) < 0.001f)
        denom = 0.001f;
    float z_factor = 1.0f / denom;
    Vec2 result = {
        v.x * z_factor * SCALE + WIDTH / 2,
        v.y * z_factor * SCALE + HEIGHT / 2};
    return result;
}

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Tesseract - Multi Rotation",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // generate all 16 vertices of the 4D hypercube
    // each vertex has coordinates +-1
    Vec4 vertices[16];
    for (int i = 0; i < 16; i++)
    {
        // for each i, assign each coordinate x, y, z, w
        // a value of either +1 or -1 depending on the state of each individual bit in i
        // i & 1 checks bit 0 for x,
        // i & 2 checks bit 1 for y...
        // If the bit is set (1), assign +1; otherwise, assign -1
        vertices[i].x = (i & 1) ? 1 : -1;
        vertices[i].y = (i & 2) ? 1 : -1;
        vertices[i].z = (i & 4) ? 1 : -1;
        vertices[i].w = (i & 8) ? 1 : -1;
    }

    int running = 1;
    SDL_Event e;
    float angle = 0.0f;

    // Print user controls
    printf("Controls:\n");
    printf("ESC - Exit\n");
    printf("0-6 - Select rotation type\n");

    int rotation_mode = 0;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = 0;
                    break;
                case SDLK_0:
                    rotation_mode = 0;
                    printf("Mode: All rotations\n");
                    break;
                case SDLK_1:
                    rotation_mode = 1;
                    printf("Mode: XY rotation\n");
                    break;
                case SDLK_2:
                    rotation_mode = 2;
                    printf("Mode: XZ rotation\n");
                    break;
                case SDLK_3:
                    rotation_mode = 3;
                    printf("Mode: XW rotation\n");
                    break;
                case SDLK_4:
                    rotation_mode = 4;
                    printf("Mode: YZ rotation\n");
                    break;
                case SDLK_5:
                    rotation_mode = 5;
                    printf("Mode: YW rotation\n");
                    break;
                case SDLK_6:
                    rotation_mode = 6;
                    printf("Mode: ZW rotation\n");
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        Vec2 projected[16];
        Vec4 rotated[16];

        for (int i = 0; i < 16; i++)
        {
            rotated[i] = vertices[i];

            switch (rotation_mode)
            {
            case 0:
                // all rotations
                rotate_xy(&rotated[i], angle * 0.7f);
                rotate_xz(&rotated[i], angle * 0.3f);
                rotate_xw(&rotated[i], angle);
                rotate_yz(&rotated[i], angle * 0.5f);
                rotate_yw(&rotated[i], angle * 0.8f);
                rotate_zw(&rotated[i], angle * 0.4f);
                break;
            case 1:
                rotate_xy(&rotated[i], angle);
                break;
            case 2:
                rotate_xz(&rotated[i], angle);
                break;
            case 3:
                rotate_xw(&rotated[i], angle);
                break;
            case 4:
                rotate_yz(&rotated[i], angle);
                break;
            case 5:
                rotate_yw(&rotated[i], angle);
                break;
            case 6:
                rotate_zw(&rotated[i], angle);
                break;
            }
        }

        // 4D -> 3D -> 2D
        for (int i = 0; i < 16; i++)
        {
            Vec3 p3 = project_4D_to_3D(rotated[i]);
            projected[i] = project_3D_to_2D(p3);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 16; i++)
        {
            for (int j = i + 1; j < 16; j++)
            {
                // two vertices are connected if they differ in exactly one coordinate
                // This is determined by counting bits in XOR of their indices
                if (__builtin_popcount(i ^ j) == 1) // __builtin_popcount returns the number of bit 1
                {
                    SDL_RenderDrawLine(renderer,
                                       (int)projected[i].x, (int)projected[i].y,
                                       (int)projected[j].x, (int)projected[j].y);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
        angle += 0.01f;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}