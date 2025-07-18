/*
    Simple two–player Pong clone with a minimal
    “victory screen”.
    — Only *new* feature compared to the previous version is the
      GAME OVER state with an on-screen message.
    — Everything else (physics, controls, dimensions) is unchanged.

    Build example (Linux):
        gcc pong.c -lSDL2 -lSDL2_ttf -lm -o pong
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> /* for rendering the victory text */

/* ---------- Game-wide constants ---------- */
#define WIDTH 900
#define HEIGHT 600

#define MOVEMENT 200.0      /* paddle speed (px/s)          */
#define BALL_MOVEMENT 350.0 /* initial ball speed (px/s)    */

#define COLOR_BLACK 0xFF000000 /* packed ARGB for SDL_Surface  */
#define MAX_ANGLE 0.509        /* max exit angle ≈ 29° (rad)   */

/* ---------- Simple types ---------- */
typedef struct
{
    double x, y, r; /* position + radius             */
    double vy, vx;  /* velocity components            */
} Circle;

typedef struct
{
    double x, y; /* top-left corner of paddle      */
} Wall;

/* Game state machine */
typedef enum
{
    PLAYING,
    GAME_OVER
} GameState;

/* ---------- Drawing helpers ---------- */
static void FillQuad(SDL_Surface *s, Wall w, Uint32 color)
{
    if (SDL_MUSTLOCK(s))
        SDL_LockSurface(s);

    SDL_Rect rect = {(int)w.x, (int)w.y, 20, 100};
    SDL_FillRect(s, &rect, color);

    if (SDL_MUSTLOCK(s))
        SDL_UnlockSurface(s);
}

static void FillCircle(SDL_Surface *s, Circle c, Uint32 color)
{
    double r2 = c.r * c.r;

    if (SDL_MUSTLOCK(s))
        SDL_LockSurface(s);

    for (double px = c.x - c.r; px <= c.x + c.r; ++px)
        for (double py = c.y - c.r; py <= c.y + c.r; ++py)
            if ((px - c.x) * (px - c.x) + (py - c.y) * (py - c.y) < r2)
            {
                SDL_Rect dot = {(int)px, (int)py, 1, 1};
                SDL_FillRect(s, &dot, color);
            }

    if (SDL_MUSTLOCK(s))
        SDL_UnlockSurface(s);
}

/* ---------- Paddle movement ---------- */
static inline void moveDown(Wall *w, double dt)
{
    w->y += MOVEMENT * dt;
    if (w->y + 100 > HEIGHT)
        w->y = HEIGHT - 100;
}
static inline void moveUp(Wall *w, double dt)
{
    w->y -= MOVEMENT * dt;
    if (w->y < 0)
        w->y = 0;
}

/* ---------- Ball physics ---------- */
static inline void updateBallPosition(Circle *c, double dt)
{
    c->x += c->vx * dt;
    c->y += c->vy * dt;

    /* bounce on top / bottom */
    if (c->y - c->r < 0)
    {
        c->y = c->r;
        c->vy = -c->vy;
    }
    if (c->y + c->r > HEIGHT)
    {
        c->y = HEIGHT - c->r;
        c->vy = -c->vy;
    }
}

static void checkCollision(Circle *c, Wall *L, Wall *R)
{
    /* paddle left */
    if (c->x - c->r <= L->x + 20 && c->x + c->r >= L->x &&
        c->y + c->r >= L->y && c->y - c->r <= L->y + 100)
    {
        double offset = (c->y - (L->y + 50.0)) / 50.0; /* −1 … +1 */
        offset = fmax(-1.0, fmin(1.0, offset));
        double angle = offset * MAX_ANGLE;
        double speed = hypot(c->vx, c->vy);

        c->vx = speed * cos(angle); /* exit to the right  */
        c->vy = speed * sin(angle);
    }
    /* paddle right */
    else if (c->x + c->r >= R->x && c->x - c->r <= R->x + 20 &&
             c->y + c->r >= R->y && c->y - c->r <= R->y + 100)
    {
        double offset = (c->y - (R->y + 50.0)) / 50.0;
        offset = fmax(-1.0, fmin(1.0, offset));
        double angle = offset * MAX_ANGLE;
        double speed = hypot(c->vx, c->vy);

        c->vx = -speed * cos(angle); /* exit to the left   */
        c->vy = speed * sin(angle);
    }
}

/* ---------- Utility to reset round ---------- */
static void resetGame(Circle *c, Wall *L, Wall *R)
{
    c->x = WIDTH / 2.0;
    c->y = HEIGHT / 2.0;
    c->vx = BALL_MOVEMENT;
    c->vy = 0.0;

    L->y = R->y = (HEIGHT - 100) / 2.0;
}

/* ---------- Main ---------- */
int main(void)
{
    /* SDL init */
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0)
    {
        fprintf(stderr, "TTF error: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Pong game",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Surface *surf = SDL_GetWindowSurface(win);
    Uint32 COLOR_WHITE = SDL_MapRGB(surf->format, 255, 255, 255);

    /* load a truetype font (provide your own .ttf in the run dir) */
    TTF_Font *font = TTF_OpenFont("FreeSansBold.otf", 48);
    if (!font)
    {
        fprintf(stderr, "Font load error: %s\n", TTF_GetError());
        return 1;
    }

    /* initial game objects */
    Circle ball = {WIDTH / 2.0, HEIGHT / 2.0, 10, 0.0, BALL_MOVEMENT};
    Wall paddleL = {40, 200};
    Wall paddleR = {WIDTH - 50.0, 200};

    GameState state = PLAYING;
    int winner = 0; /* -1 left, +1 right */

    Uint32 prev = SDL_GetTicks();
    int running = 1;
    SDL_Event e;

    while (running)
    {

        /* ---------------- EVENTS ---------------- */
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
            if (state == GAME_OVER && e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_r)
                { /* restart */
                    resetGame(&ball, &paddleL, &paddleR);
                    state = PLAYING;
                }
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    running = 0;
            }
        }

        /* time step */
        Uint32 now = SDL_GetTicks();
        double dt = (now - prev) / 1000.0;
        prev = now;

        /* ------------- GAME LOGIC --------------- */
        if (state == PLAYING)
        {

            /* keyboard state */
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_W])
                moveUp(&paddleL, dt);
            if (keys[SDL_SCANCODE_S])
                moveDown(&paddleL, dt);
            if (keys[SDL_SCANCODE_O])
                moveUp(&paddleR, dt);
            if (keys[SDL_SCANCODE_L])
                moveDown(&paddleR, dt);

            updateBallPosition(&ball, dt);
            checkCollision(&ball, &paddleL, &paddleR);

            /* check out-of-bounds (score) */
            if (ball.x - ball.r < 0)
            {
                state = GAME_OVER;
                winner = +1;
            }
            else if (ball.x + ball.r > WIDTH)
            {
                state = GAME_OVER;
                winner = -1;
            }
        }

        /* ------------- RENDER ------------------- */
        SDL_FillRect(surf, NULL, COLOR_BLACK);

        if (state == PLAYING)
        {
            FillQuad(surf, paddleL, COLOR_WHITE);
            FillQuad(surf, paddleR, COLOR_WHITE);
            FillCircle(surf, ball, COLOR_WHITE);
        }
        else
        { /* GAME_OVER */
            const char *msg = (winner == -1)
                                  ? "LEFT PLAYER WINS!"
                                  : "RIGHT PLAYER WINS!";

            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *text = TTF_RenderUTF8_Blended(font, msg, white);

            SDL_Rect dst = {(WIDTH - text->w) / 2, (HEIGHT - text->h) / 2,
                            text->w, text->h};
            SDL_BlitSurface(text, NULL, surf, &dst);
            SDL_FreeSurface(text);

            /* small hint line */
            SDL_Surface *hint = TTF_RenderUTF8_Blended(font,
                                                       "Press R to restart — Esc to quit", white);
            dst.x = (WIDTH - hint->w) / 2;
            dst.y = HEIGHT / 2 + text->h;
            SDL_BlitSurface(hint, NULL, surf, &dst);
            SDL_FreeSurface(hint);
        }

        SDL_UpdateWindowSurface(win);
        SDL_Delay(16); /* ~60 fps cap */
    }

    /* ---------- cleanup ---------- */
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}