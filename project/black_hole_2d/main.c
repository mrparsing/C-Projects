#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

#define C_SPEED 30.0
#define NUM_RAYS 100
#define TIME_STEP 0.05
#define MAX_TRAIL_POINTS 1000

typedef struct
{
    double x, y, z;
} Vector3;

typedef struct
{
    Vector3 position;
    double schwarzschild_radius;
} BlackHole;

typedef struct
{
    double x, y;
    double r, phi;
    double dr, dphi;
    Vector3 direction;
    Vector3 trail[MAX_TRAIL_POINTS];
    int trail_head;
    int trail_length;
} Ray;

typedef struct
{
    Ray ray[NUM_RAYS];
} ArrayRay;

typedef struct
{
    double r, phi;
    double dr, dphi;
} State;

void updatePolarCoordinates(Ray *ray, const BlackHole *bh);
void derivatives(const State *s, double rs, State *ds);
void rk4_step(State *s, double h, double rs);
void drawBlackHole(const BlackHole *bh);
void drawRay(const Ray *ray);
void drawTrail(const Ray *ray);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int main(void)
{
    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(800, 600, "Black Hole", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    BlackHole blackhole = {
        .position = {width / 2.0, height / 2.0, 0.0},
        .schwarzschild_radius = 30.0};

    ArrayRay rays;
    for (int i = 0; i < NUM_RAYS; i++)
    {
        Ray *r = &rays.ray[i];
        r->x = 50.0;
        r->y = i * 12.0;

        r->trail_head = 0;
        r->trail_length = 0;
        for (int j = 0; j < MAX_TRAIL_POINTS; j++)
        {
            r->trail[j] = (Vector3){r->x, r->y, 0.0};
        }

        r->direction = (Vector3){1.0, 0.0, 0.0};
        updatePolarCoordinates(r, &blackhole);

        double vx = r->direction.x * C_SPEED;
        double vy = r->direction.y * C_SPEED;
        double dx = r->x - blackhole.position.x;
        double dy = r->y - blackhole.position.y;

        r->dr = (dx * vx + dy * vy) / r->r;
        r->dphi = (dx * vy - dy * vx) / (r->r * r->r);
    }

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        drawBlackHole(&blackhole);

        for (int i = 0; i < NUM_RAYS; i++)
        {
            Ray *r = &rays.ray[i];

            r->trail[r->trail_head] = (Vector3){r->x, r->y, 0.0};
            r->trail_head = (r->trail_head + 1) % MAX_TRAIL_POINTS;
            if (r->trail_length < MAX_TRAIL_POINTS)
                r->trail_length++;

            // if the ray is too close to the black hole, do not update it but continue drawing the trail
            if (r->r > blackhole.schwarzschild_radius * 1.5)
            {
                State s = {r->r, r->phi, r->dr, r->dphi};
                rk4_step(&s, TIME_STEP, blackhole.schwarzschild_radius);

                r->r = s.r;
                r->phi = s.phi;
                r->dr = s.dr;
                r->dphi = s.dphi;

                r->x = blackhole.position.x + r->r * cos(r->phi);
                r->y = blackhole.position.y + r->r * sin(r->phi);

                double vr = r->dr;
                double vphi = r->dphi;
                r->direction.x = vr * cos(r->phi) - r->r * vphi * sin(r->phi);
                r->direction.y = vr * sin(r->phi) + r->r * vphi * cos(r->phi);

                double len = hypot(r->direction.x, r->direction.y);
                if (len > 0)
                {
                    r->direction.x /= len;
                    r->direction.y /= len;
                }
            }
            drawTrail(r);
            if (r->r > blackhole.schwarzschild_radius * 1.5)
                drawRay(r);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void updatePolarCoordinates(Ray *ray, const BlackHole *bh)
{
    double dx = ray->x - bh->position.x;
    double dy = ray->y - bh->position.y;
    ray->r = hypot(dx, dy);
    ray->phi = atan2(dy, dx);
}

void derivatives(const State *s, double rs, State *ds)
{
    double r = s->r;

    if (r < rs * 1.1)
    {
        *ds = (State){0, 0, 0, 0};
        return;
    }

    double A = 1.0 - rs / r;
    double dA_dr = rs / (r * r);

    ds->r = s->dr;
    ds->phi = s->dphi;
    ds->dr = r * A * pow(s->dphi, 2) - (dA_dr / (2 * A)) * pow(s->dr, 2) - (C_SPEED * C_SPEED * dA_dr) / (2 * A);
    ds->dphi = -2.0 * s->dr * s->dphi / r;
}

void rk4_step(State *s, double h, double rs)
{
    State k1, k2, k3, k4, temp;

    derivatives(s, rs, &k1);

    temp.r = s->r + 0.5 * h * k1.r;
    temp.phi = s->phi + 0.5 * h * k1.phi;
    temp.dr = s->dr + 0.5 * h * k1.dr;
    temp.dphi = s->dphi + 0.5 * h * k1.dphi;
    derivatives(&temp, rs, &k2);

    temp.r = s->r + 0.5 * h * k2.r;
    temp.phi = s->phi + 0.5 * h * k2.phi;
    temp.dr = s->dr + 0.5 * h * k2.dr;
    temp.dphi = s->dphi + 0.5 * h * k2.dphi;
    derivatives(&temp, rs, &k3);

    temp.r = s->r + h * k3.r;
    temp.phi = s->phi + h * k3.phi;
    temp.dr = s->dr + h * k3.dr;
    temp.dphi = s->dphi + h * k3.dphi;
    derivatives(&temp, rs, &k4);

    s->r += h * (k1.r + 2 * k2.r + 2 * k3.r + k4.r) / 6.0;
    s->phi += h * (k1.phi + 2 * k2.phi + 2 * k3.phi + k4.phi) / 6.0;
    s->dr += h * (k1.dr + 2 * k2.dr + 2 * k3.dr + k4.dr) / 6.0;
    s->dphi += h * (k1.dphi + 2 * k2.dphi + 2 * k3.dphi + k4.dphi) / 6.0;
}

void drawBlackHole(const BlackHole *bh)
{
    // accretion disk
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 100; i++)
    {
        float angle = 2.0f * M_PI * i / 100;
        float x = cos(angle);
        float y = sin(angle);

        // Inner disk (closer to event horizon)
        glColor4f(1.0, 0.5, 0.0, 0.7); // orange
        glVertex2f(
            bh->position.x + x * bh->schwarzschild_radius * 1.5,
            bh->position.y + y * bh->schwarzschild_radius * 1.5);

        // Outer disk
        glColor4f(0.5, 0.2, 0.8, 0.4); // purple
        glVertex2f(
            bh->position.x + x * bh->schwarzschild_radius * 3.0,
            bh->position.y + y * bh->schwarzschild_radius * 3.0);
    }
    glEnd();

    // Draw event horizon (black circle)
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(bh->position.x, bh->position.y);
    for (int i = 0; i <= 100; i++)
    {
        float angle = 2.0f * M_PI * i / 100;
        glVertex2f(
            bh->position.x + cos(angle) * bh->schwarzschild_radius,
            bh->position.y + sin(angle) * bh->schwarzschild_radius);
    }
    glEnd();

    // Draw photon sphere (dashed line)
    glColor3f(0.5, 0.5, 1.0);
    glLineStipple(1, 0x00FF); // dashed pattern
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        float angle = 2.0f * M_PI * i / 100;
        glVertex2f(
            bh->position.x + cos(angle) * bh->schwarzschild_radius * 1.5,
            bh->position.y + sin(angle) * bh->schwarzschild_radius * 1.5);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

void drawRay(const Ray *ray)
{
    double length = 20.0;
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(ray->x, ray->y);
    glVertex2f(ray->x + ray->direction.x * length, ray->y + ray->direction.y * length);
    glEnd();
}

void drawTrail(const Ray *ray)
{
    if (ray->trail_length < 2)
        return;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < ray->trail_length; i++)
    {
        int idx = (ray->trail_head - ray->trail_length + i + MAX_TRAIL_POINTS) % MAX_TRAIL_POINTS;

        float alpha = (float)i / ray->trail_length;
        glColor4f(1.0, 1.0, 0.0, alpha * 0.7);

        glVertex2f(ray->trail[idx].x, ray->trail[idx].y);
    }
    glEnd();
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
}