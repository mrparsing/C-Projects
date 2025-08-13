#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <GLUT/glut.h>

#define C_SPEED 1.0
#define WIDTH 400
#define HEIGHT 300
#define NUM_RAYS 1000
#define TIME_STEP 0.05
#define MAX_TRAIL_POINTS 300
#define MAX_STEPS 20000

#define DISK_INNER_RADIUS (1.5 * blackhole.schwarzschild_radius)
#define DISK_OUTER_RADIUS (10.0 * blackhole.schwarzschild_radius)
#define DISK_THICKNESS 0.1
#define DISK_TEMPERATURE 1e6

// LOD distances for performance optimization
#define LOD_DISTANCE_1 50.0
#define LOD_DISTANCE_2 100.0

typedef struct
{
    double x, y, z;
} Vector3;

typedef struct
{
    Vector3 position;
    double schwarzschild_radius;
    double mass;
} BlackHole;

typedef struct
{
    float r, g, b, a;
} Color;

typedef struct
{
    Vector3 position;
    double r, theta, phi;
    double dr, dtheta, dphi;
    double energy;
    double angular_momentum;
    Vector3 direction;
    Vector3 trail[MAX_TRAIL_POINTS];
    int trail_head;
    int trail_length;
    Color color;
    int active;
    int absorbed;  // flag to indicate if ray was absorbed
    int fade_timer; // timer for fading out trail after absorption
} Ray;

typedef struct
{
    Ray rays[NUM_RAYS];
} ArrayRay;

typedef struct
{
    double r, theta, phi;
    double dr, dtheta, dphi;
    double energy;
    double angular_momentum;
} State;

typedef struct
{
    float posX, posY, posZ;
    float yaw, pitch, roll;
    float frontX, frontY, frontZ;
    float rightX, rightY, rightZ;
    float upX, upY, upZ;
    float lastX, lastY;
    int firstMouse;
    float mouseSensitivity;
    float movementSpeed;
} Camera;

typedef struct
{
    Vector3 pos;
    Vector3 front;
    Vector3 up;
    float fov;
} Emitter;

Camera camera = {
    .posX = 70.0f, .posY = 106.0f, .posZ = -200.0f, .yaw = 110.0f, .pitch = -18.0f, .roll = 0.0f, .firstMouse = 1, .mouseSensitivity = 0.1f, .movementSpeed = 0.15f};

static int mouseEnabled = 1;
static int showHelp = 0;
static int showInfoPanel = 1;
static int showGrid = 1;
static int showAccretionDisk = 1;
static int showLensing = 1;
static int paused = 0;
static double lastFrameTime = 0.0;
static float deltaTime = 0.0f;
static float time_dilation_factor = 1.0f;
static float schwarzschild_radius = 3.0f;
static int ray_count = NUM_RAYS;
static int frame_count = 0;
static int current_scenario = 0;
static int reset = 0;

void updatePolarCoordinates(Ray *ray, const BlackHole *bh);
void geodesic_derivatives(const State *s, double rs, State *ds);
void rk4_step(State *s, double h, double rs);
void drawBlackHole(const BlackHole *bh);
void drawEmitter(const Emitter *emitter);
void drawSpacetimeGrid(const BlackHole *bh);
void drawDistortedGrid(const BlackHole *bh);
void drawAccretionDisk(const BlackHole *bh);
void drawRay(const Ray *ray);
void drawTrail(const Ray *ray);
void drawAdvancedTrail(const Ray *ray, const BlackHole *bh);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void updateCameraVectors();
void generateRays(const Emitter *emitter, Ray *rays, int num_rays, const BlackHole *blackhole);
void initEmitter(Emitter *emitter);
void initEmitter2(Emitter *emitter);
void drawSphere(float cx, float cy, float cz, float radius, int slices, int stacks);
void myLookAt(float eyeX, float eyeY, float eyeZ,
              float centerX, float centerY, float centerZ,
              float upX, float upY, float upZ);
void processInput(GLFWwindow *window);
void drawHelpText();
void drawInfoPanel();
void drawTextLine(int x, int y, int length);
void drawNumericBar(float value, int x, int y, int max_bars);
void toggleMouseControl(GLFWwindow *window);
double calculateTimeDilation(double r, double rs);
void updateRaysWithRelativity(Ray *rays, int num_rays, const BlackHole *bh, double dt);
void updateCartesianFromPolar(Ray *r, const BlackHole *bh);
double distanceToCamera(const Ray *ray, const Camera *cam);
void updateRaysLOD(Ray *rays, int num_rays, const Camera *cam, const BlackHole *bh);
void resetSimulation(Emitter *emitter, Emitter *emitter2, Ray *rays, const BlackHole *bh);
void drawProgressBar(float value, int x, int y, int width, float r, float g, float b);
void drawGLUTText(float x, float y, void *font, const char *string);
void drawScaledGLUTText(float x, float y, void *font, const char *string, float scale);

int main(void)
{
    srand(time(NULL));
    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(1200, 800, "Black Hole Simulator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    BlackHole blackhole = {
        .position = {0.0, 30.0, 0.0},
        .schwarzschild_radius = schwarzschild_radius,
        .mass = schwarzschild_radius / 2.0};

    Emitter emitter;
    Emitter emitter2;
    initEmitter(&emitter);
    initEmitter2(&emitter2);

    ArrayRay arrayRays;
    generateRays(&emitter, arrayRays.rays, NUM_RAYS / 2, &blackhole);
    generateRays(&emitter2, arrayRays.rays + NUM_RAYS / 2, NUM_RAYS / 2, &blackhole);

    // Initialize ray physics
    for (int i = 0; i < NUM_RAYS; i++)
    {
        Ray *r = &arrayRays.rays[i];
        updatePolarCoordinates(r, &blackhole);

        double vx = r->direction.x * C_SPEED;
        double vy = r->direction.y * C_SPEED;
        double vz = r->direction.z * C_SPEED;

        double sin_theta = sin(r->theta);
        double cos_theta = cos(r->theta);
        double sin_phi = sin(r->phi);
        double cos_phi = cos(r->phi);

        r->dr = sin_theta * cos_phi * vx + sin_theta * sin_phi * vy + cos_theta * vz;
        r->dtheta = (cos_theta * cos_phi * vx + cos_theta * sin_phi * vy - sin_theta * vz) / r->r;
        r->dphi = (-sin_phi * vx + cos_phi * vy) / (r->r * sin_theta);

        // Calculate conserved quantities
        double A = 1.0 - blackhole.schwarzschild_radius / r->r;
        r->energy = A * (1.0 + r->dr * r->dr / A); // simplified energy
        r->angular_momentum = r->r * r->r * sin_theta * sin_theta * r->dphi;

        r->active = 1;
        r->absorbed = 0;
        r->fade_timer = 0;
    }

    updateCameraVectors();
    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;
        frame_count++;

        processInput(window);

        // Update black hole parameters
        blackhole.schwarzschild_radius = schwarzschild_radius;
        blackhole.mass = schwarzschild_radius / 2.0;

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup perspective projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)width / (float)height;
        float fov = 45.0f;
        float near = 0.1f;
        float far = 1000.0f;
        float range = tan(fov * 0.5f * M_PI / 180.0f) * near;
        float left = -range * aspect;
        float right = range * aspect;
        float bottom = -range;
        float top = range;
        glFrustum(left, right, bottom, top, near, far);

        // Setup camera view
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        printf("1) %f - %f - %f\n", camera.posX, camera.posY, camera.posZ);
        printf("2) %f - %f - %f\n\n", camera.yaw, camera.pitch, camera.roll);
        myLookAt(
            camera.posX, camera.posY, camera.posZ,
            camera.posX + camera.frontX,
            camera.posY + camera.frontY,
            camera.posZ + camera.frontZ,
            camera.upX, camera.upY, camera.upZ);

        // Draw scene elements
        drawBlackHole(&blackhole);

        if (showGrid)
        {
            if (showLensing)
            {
                drawDistortedGrid(&blackhole);
            }
            else
            {
                drawSpacetimeGrid(&blackhole);
            }
        }

        if (showAccretionDisk)
        {
            drawAccretionDisk(&blackhole);
        }

        drawEmitter(&emitter);
        drawEmitter(&emitter2);

        // Update and draw rays
        if (!paused)
        {
            updateRaysWithRelativity(arrayRays.rays, ray_count, &blackhole, TIME_STEP);
        }

        for (int i = 0; i < ray_count; i++)
        {
            Ray *r = &arrayRays.rays[i];
            
            // Always draw trail, even for absorbed rays (until fade completes)
            if (r->trail_length > 1)
            {
                drawAdvancedTrail(r, &blackhole);
            }
            
            // Only draw the ray itself if it's still active and not absorbed
            if (r->active && !r->absorbed && r->r > blackhole.schwarzschild_radius * 1.1)
            {
                drawRay(r);
            }
        }

        // Draw UI elements
        if (showInfoPanel)
        {
            drawInfoPanel();
        }

        if (showHelp)
        {
            drawHelpText();
        }

        if (reset)
        {
            resetSimulation(&emitter, &emitter2, &arrayRays.rays, &blackhole);
            reset = 0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void updateRaysWithRelativity(Ray *rays, int num_rays, const BlackHole *bh, double dt)
{
    for (int i = 0; i < num_rays; i++)
    {
        Ray *r = &rays[i];
        
        // Handle absorbed rays - let them fade out gradually
        if (r->absorbed)
        {
            r->fade_timer++;
            if (r->fade_timer > 120)  // Fade for 120 frames (~2 seconds at 60fps)
            {
                r->active = 0;  // Finally deactivate completely
            }
            continue;  // Skip physics update for absorbed rays
        }
        
        if (!r->active)
            continue;

        // Add to trail only if ray is still active
        r->trail[r->trail_head] = r->position;
        r->trail_head = (r->trail_head + 1) % MAX_TRAIL_POINTS;
        if (r->trail_length < MAX_TRAIL_POINTS)
            r->trail_length++;

        // Check if ray is approaching event horizon
        if (r->r <= bh->schwarzschild_radius * 1.05)  // Very close to event horizon
        {
            r->absorbed = 1;  // Mark as absorbed but don't deactivate yet
            r->fade_timer = 0;
            continue;
        }

        // Calculate local time dilation
        double dilation = calculateTimeDilation(r->r, bh->schwarzschild_radius);
        double effective_dt = dt * dilation * time_dilation_factor;

        // Update physics if far enough from event horizon
        if (r->r > bh->schwarzschild_radius * 1.1)
        {
            State s = {r->r, r->theta, r->phi, r->dr, r->dtheta, r->dphi, r->energy, r->angular_momentum};
            rk4_step(&s, effective_dt, bh->schwarzschild_radius);

            r->r = s.r;
            r->theta = s.theta;
            r->phi = s.phi;
            r->dr = s.dr;
            r->dtheta = s.dtheta;
            r->dphi = s.dphi;

            updateCartesianFromPolar(r, bh);

            // Update direction vector
            double sin_theta = sin(r->theta);
            double cos_theta = cos(r->theta);
            double sin_phi = sin(r->phi);
            double cos_phi = cos(r->phi);

            double vx = r->dr * sin_theta * cos_phi + r->r * cos_theta * cos_phi * r->dtheta - r->r * sin_theta * sin_phi * r->dphi;
            double vy = r->dr * sin_theta * sin_phi + r->r * cos_theta * sin_phi * r->dtheta + r->r * sin_theta * cos_phi * r->dphi;
            double vz = r->dr * cos_theta - r->r * sin_theta * r->dtheta;

            double len = sqrt(vx * vx + vy * vy + vz * vz);
            if (len > 0)
            {
                r->direction.x = vx / len;
                r->direction.y = vy / len;
                r->direction.z = vz / len;
            }
        }

        // Deactivate rays that are too far away
        if (r->r > 200.0)
        {
            r->active = 0;
        }
    }
}

void drawAdvancedTrail(const Ray *ray, const BlackHole *bh)
{
    if (ray->trail_length < 2)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < ray->trail_length - 1; i++)
    {
        int idx = (ray->trail_head - ray->trail_length + i + MAX_TRAIL_POINTS) % MAX_TRAIL_POINTS;
        int next_idx = (ray->trail_head - ray->trail_length + i + 1 + MAX_TRAIL_POINTS) % MAX_TRAIL_POINTS;

        // Calculate velocity for Doppler effect simulation
        Vector3 velocity;
        velocity.x = ray->trail[next_idx].x - ray->trail[idx].x;
        velocity.y = ray->trail[next_idx].y - ray->trail[idx].y;
        velocity.z = ray->trail[next_idx].z - ray->trail[idx].z;

        double speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);

        // Calculate distance to black hole for redshift effect
        double dx = ray->trail[idx].x - bh->position.x;
        double dy = ray->trail[idx].y - bh->position.y;
        double dz = ray->trail[idx].z - bh->position.z;
        double dist_to_bh = sqrt(dx * dx + dy * dy + dz * dz);

        // Gravitational redshift factor
        double redshift_factor = 1.0 - bh->schwarzschild_radius / (2.0 * dist_to_bh);

        // Color based on velocity and redshift
        float red = 1.0f - speed * 0.3f + (1.0f - redshift_factor) * 2.0f;
        float green = 0.5f + speed * 0.2f;
        float blue = speed + redshift_factor * 0.5f;
        float alpha = (float)i / ray->trail_length;

        // Apply fading for absorbed rays
        if (ray->absorbed && ray->fade_timer > 0)
        {
            float fade_factor = 1.0f - ((float)ray->fade_timer / 120.0f);
            alpha *= fade_factor;
            red *= fade_factor;
            green *= fade_factor;
            blue *= fade_factor;
        }

        red = fmax(0.0f, fmin(1.0f, red));
        green = fmax(0.0f, fmin(1.0f, green));
        blue = fmax(0.0f, fmin(1.0f, blue));

        glColor4f(red, green, blue, alpha * 0.9f);
        glVertex3f(ray->trail[idx].x, ray->trail[idx].y, ray->trail[idx].z);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void resetSimulation(Emitter *emitter, Emitter *emitter2, Ray *rays, const BlackHole *bh)
{
    // Reset emitter to default position
    initEmitter(emitter);
    initEmitter2(emitter2);

    // Regenerate and reinitialize rays for both emitters
    generateRays(emitter, rays, NUM_RAYS / 2, bh);
    generateRays(emitter2, rays + NUM_RAYS / 2, NUM_RAYS / 2, bh);

    // Reinitialize ray physics for all rays
    for (int i = 0; i < NUM_RAYS; i++)
    {
        Ray *r = &rays[i];
        updatePolarCoordinates(r, bh);

        double vx = r->direction.x * C_SPEED;
        double vy = r->direction.y * C_SPEED;
        double vz = r->direction.z * C_SPEED;

        double sin_theta = sin(r->theta);
        double cos_theta = cos(r->theta);
        double sin_phi = sin(r->phi);
        double cos_phi = cos(r->phi);

        r->dr = sin_theta * cos_phi * vx + sin_theta * sin_phi * vy + cos_theta * vz;
        r->dtheta = (cos_theta * cos_phi * vx + cos_theta * sin_phi * vy - sin_theta * vz) / r->r;
        r->dphi = (-sin_phi * vx + cos_phi * vy) / (r->r * sin_theta);

        // Calculate conserved quantities
        double A = 1.0 - bh->schwarzschild_radius / r->r;
        r->energy = A * (1.0 + r->dr * r->dr / A);
        r->angular_momentum = r->r * r->r * sin_theta * sin_theta * r->dphi;

        r->active = 1;
        r->absorbed = 0;  // Reset absorption flag
        r->fade_timer = 0; // Reset fade timer
        r->trail_length = 0;
        r->trail_head = 0;
    }

    // Reset simulation parameters
    time_dilation_factor = 1.0f;
    paused = 0;

    printf("Simulation reset to initial state\n");
}

void processInput(GLFWwindow *window)
{
    float baseSpeed = camera.movementSpeed * deltaTime * 60.0f;
    float currentSpeed = baseSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentSpeed *= 3.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        currentSpeed *= 0.3f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.posX += camera.frontX * currentSpeed;
        camera.posY += camera.frontY * currentSpeed;
        camera.posZ += camera.frontZ * currentSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.posX -= camera.frontX * currentSpeed;
        camera.posY -= camera.frontY * currentSpeed;
        camera.posZ -= camera.frontZ * currentSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.posX -= camera.rightX * currentSpeed;
        camera.posY -= camera.rightY * currentSpeed;
        camera.posZ -= camera.rightZ * currentSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.posX += camera.rightX * currentSpeed;
        camera.posY += camera.rightY * currentSpeed;
        camera.posZ += camera.rightZ * currentSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera.posX += camera.upX * currentSpeed;
        camera.posY += camera.upY * currentSpeed;
        camera.posZ += camera.upZ * currentSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        camera.posX -= camera.upX * currentSpeed;
        camera.posY -= camera.upY * currentSpeed;
        camera.posZ -= camera.upZ * currentSpeed;
    }

    if (!mouseEnabled)
    {
        float rotSpeed = 90.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.yaw -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.yaw += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.pitch += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.pitch -= rotSpeed;

        if (camera.pitch > 89.0f)
            camera.pitch = 89.0f;
        if (camera.pitch < -89.0f)
            camera.pitch = -89.0f;
        updateCameraVectors();
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_TAB:
            toggleMouseControl(window);
            break;
        case GLFW_KEY_R:
            camera.posX = 70.0f;
            camera.posY = 106.0f;
            camera.posZ = -200.0f;
            camera.yaw = 110.0f;
            camera.pitch = -18.0f;
            camera.roll = 0.0f;
            updateCameraVectors();
            printf("Camera reset\n");
            break;
        case GLFW_KEY_H:
            showHelp = !showHelp;
            printf("Help %s\n", showHelp ? "ON" : "OFF");
            break;
        case GLFW_KEY_I:
            showInfoPanel = !showInfoPanel;
            printf("Info panel %s\n", showInfoPanel ? "ON" : "OFF");
            break;
        case GLFW_KEY_P:
            paused = !paused;
            printf("Simulation %s\n", paused ? "PAUSED" : "RUNNING");
            break;
        case GLFW_KEY_T:
            time_dilation_factor = fmodf(time_dilation_factor + 0.2f, 2.2f);
            printf("Time dilation: %.1fx\n", time_dilation_factor);
            break;
        case GLFW_KEY_G:
            showGrid = !showGrid;
            printf("Spacetime grid %s\n", showGrid ? "ON" : "OFF");
            break;
        case GLFW_KEY_L:
            showLensing = !showLensing;
            printf("Gravitational lensing %s\n", showLensing ? "ON" : "OFF");
            break;
        case GLFW_KEY_UP:
            if (schwarzschild_radius < 4.8f)
            {
                schwarzschild_radius += 0.2f;
                printf("Black hole mass increased - Rs: %.1f\n", schwarzschild_radius);
            }
            break;
        case GLFW_KEY_DOWN:
            if (schwarzschild_radius > 3.0f)
            {
                schwarzschild_radius -= 0.2f;
                printf("Black hole mass decreased - Rs: %.1f\n", schwarzschild_radius);
            }
            break;
        case GLFW_KEY_ENTER:
            printf("Resetting simulation\n");
            reset = 1;
            break;
        }
    }
}

void geodesic_derivatives(const State *s, double rs, State *ds)
{
    double r = s->r;
    double theta = s->theta;
    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    if (r < rs * 1.01)
    {
        *ds = (State){0, 0, 0, 0, 0, 0, 0};
        return;
    }

    double A = 1.0 - rs / r;
    double sin2_theta = sin_theta * sin_theta;
    double E = s->energy;
    double L = s->angular_momentum;

    ds->r = s->dr;
    ds->theta = s->dtheta;
    ds->phi = s->dphi;

    // Enhanced radial equation with energy and angular momentum conservation
    ds->dr = -rs / (2 * r * r * A) * (E * E / A - 1) + L * L / (r * r * r) - rs * L * L / (2 * r * r * r * r);

    // Angular equations
    if (sin_theta > 1e-10)
    {
        ds->dtheta = L * cos_theta / (r * r * sin2_theta * sin_theta);
        ds->dphi = L / (r * r * sin2_theta);
    }
    else
    {
        ds->dtheta = 0;
        ds->dphi = 0;
    }

    ds->energy = 0;           // Energy is conserved
    ds->angular_momentum = 0; // Angular momentum is conserved
}

void rk4_step(State *s, double h, double rs)
{
    State k1, k2, k3, k4, temp;

    geodesic_derivatives(s, rs, &k1);

    temp.r = s->r + 0.5 * h * k1.r;
    temp.theta = s->theta + 0.5 * h * k1.theta;
    temp.phi = s->phi + 0.5 * h * k1.phi;
    temp.dr = s->dr + 0.5 * h * k1.dr;
    temp.dtheta = s->dtheta + 0.5 * h * k1.dtheta;
    temp.dphi = s->dphi + 0.5 * h * k1.dphi;
    temp.energy = s->energy;
    temp.angular_momentum = s->angular_momentum;
    geodesic_derivatives(&temp, rs, &k2);

    temp.r = s->r + 0.5 * h * k2.r;
    temp.theta = s->theta + 0.5 * h * k2.theta;
    temp.phi = s->phi + 0.5 * h * k2.phi;
    temp.dr = s->dr + 0.5 * h * k2.dr;
    temp.dtheta = s->dtheta + 0.5 * h * k2.dtheta;
    temp.dphi = s->dphi + 0.5 * h * k2.dphi;
    temp.energy = s->energy;
    temp.angular_momentum = s->angular_momentum;
    geodesic_derivatives(&temp, rs, &k3);

    temp.r = s->r + h * k3.r;
    temp.theta = s->theta + h * k3.theta;
    temp.phi = s->phi + h * k3.phi;
    temp.dr = s->dr + h * k3.dr;
    temp.dtheta = s->dtheta + h * k3.dtheta;
    temp.dphi = s->dphi + h * k3.dphi;
    temp.energy = s->energy;
    temp.angular_momentum = s->angular_momentum;
    geodesic_derivatives(&temp, rs, &k4);

    s->r += h * (k1.r + 2 * k2.r + 2 * k3.r + k4.r) / 6.0;
    s->theta += h * (k1.theta + 2 * k2.theta + 2 * k3.theta + k4.theta) / 6.0;
    s->phi += h * (k1.phi + 2 * k2.phi + 2 * k3.phi + k4.phi) / 6.0;
    s->dr += h * (k1.dr + 2 * k2.dr + 2 * k3.dr + k4.dr) / 6.0;
    s->dtheta += h * (k1.dtheta + 2 * k2.dtheta + 2 * k3.dtheta + k4.dtheta) / 6.0;
    s->dphi += h * (k1.dphi + 2 * k2.dphi + 2 * k3.dphi + k4.dphi) / 6.0;
}

double calculateTimeDilation(double r, double rs)
{
    if (r <= rs)
        return 0.0;
    return sqrt(1.0 - rs / r);
}

void updateCartesianFromPolar(Ray *r, const BlackHole *bh)
{
    r->position.x = bh->position.x + r->r * sin(r->theta) * cos(r->phi);
    r->position.y = bh->position.y + r->r * sin(r->theta) * sin(r->phi);
    r->position.z = bh->position.z + r->r * cos(r->theta);
}

void drawAccretionDisk(const BlackHole *bh)
{
    double rs = bh->schwarzschild_radius;
    double inner_radius = 3.0 * rs; // ISCO
    double outer_radius = 15.0 * rs;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int ring = 0; ring < 50; ring++)
    {
        double r1 = inner_radius + (outer_radius - inner_radius) * ring / 50.0;
        double r2 = inner_radius + (outer_radius - inner_radius) * (ring + 1) / 50.0;

        // Temperature-based coloring
        double temp_factor = inner_radius / r1;
        float red = fmin(1.0f, temp_factor * 2.0f);
        float green = fmin(1.0f, temp_factor);
        float blue = fmin(0.5f, temp_factor * 0.5f);
        float alpha = 0.6f * (1.0f - (float)ring / 50.0f);

        glColor4f(red, green, blue, alpha);

        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= 72; i++)
        {
            double angle = i * M_PI / 36.0;
            double x1 = cos(angle) * r1;
            double z1 = sin(angle) * r1;
            double x2 = cos(angle) * r2;
            double z2 = sin(angle) * r2;

            // Add slight height variation for disk thickness
            double h = 0.1 * sin(angle * 8.0) * exp(-r1 / outer_radius);

            glVertex3f(bh->position.x + x1, bh->position.y + h, bh->position.z + z1);
            glVertex3f(bh->position.x + x2, bh->position.y + h, bh->position.z + z2);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
}

void drawDistortedGrid(const BlackHole *bh)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);

    int num_lines = 60;
    double grid_min = -100.0;
    double grid_max = 100.0;
    double step = (grid_max - grid_min) / num_lines;

    // Radial grid lines with gravitational lensing effect
    for (int i = 0; i <= num_lines; i++)
    {
        double z = grid_min + i * step;
        glBegin(GL_LINE_STRIP);

        for (int j = 0; j <= 150; j++)
        {
            double x = grid_min + (grid_max - grid_min) * (double)j / 150.0;
            double dx = x - bh->position.x;
            double dz = z - bh->position.z;
            double r = sqrt(dx * dx + dz * dz);

            if (r < bh->schwarzschild_radius * 2.0)
                continue;

            // Calculate gravitational lensing deflection
            double deflection = 4.0 * bh->schwarzschild_radius / r;
            double curvature_factor = 5.0;
            double y = -bh->position.y + curvature_factor * sqrt(bh->schwarzschild_radius * (r - bh->schwarzschild_radius));

            // Apply lensing distortion
            double bent_factor = 1.0 + deflection * 0.5;
            y *= bent_factor;

            // Color based on curvature intensity
            float intensity = fmin(1.0f, (float)(bh->schwarzschild_radius * 10.0 / r));
            glColor4f(0.2f + intensity * 0.5f, 0.2f + intensity * 0.3f, 0.8f, 0.6f);

            glVertex3d(x, y, z);
        }
        glEnd();
    }

    // Circular grid lines
    for (int i = 0; i <= num_lines; i++)
    {
        double x = grid_min + i * step;
        glBegin(GL_LINE_STRIP);

        for (int j = 0; j <= 150; j++)
        {
            double z = grid_min + (grid_max - grid_min) * (double)j / 150.0;
            double dx = x - bh->position.x;
            double dz = z - bh->position.z;
            double r = sqrt(dx * dx + dz * dz);

            if (r < bh->schwarzschild_radius * 2.0)
                continue;

            double deflection = 4.0 * bh->schwarzschild_radius / r;
            double curvature_factor = 5.0;
            double y = -bh->position.y + curvature_factor * sqrt(bh->schwarzschild_radius * (r - bh->schwarzschild_radius));

            double bent_factor = 1.0 + deflection * 0.5;
            y *= bent_factor;

            float intensity = fmin(1.0f, (float)(bh->schwarzschild_radius * 10.0 / r));
            glColor4f(0.2f + intensity * 0.5f, 0.2f + intensity * 0.3f, 0.8f, 0.6f);

            glVertex3d(x, y, z);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
}

void drawTextLine(int x, int y, int length)
{
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < length * 8; i++)
    {
        if ((i / 8) % 2 == 0)
        { // Create text-like pattern
            glVertex2f(x + i, y);
            glVertex2f(x + i, y + 2);
        }
    }
    glEnd();
}

void drawNumericBar(float value, int x, int y, int max_bars)
{
    int bars = (int)(value * max_bars);
    glBegin(GL_QUADS);
    for (int i = 0; i < bars && i < max_bars; i++)
    {
        float intensity = 1.0f - (float)i / max_bars;
        glColor3f(1.0f - intensity * 0.5f, intensity, 0.2f);

        glVertex2f(x + i * 4, y);
        glVertex2f(x + i * 4 + 3, y);
        glVertex2f(x + i * 4 + 3, y + 8);
        glVertex2f(x + i * 4, y + 8);
    }
    glEnd();
}

void drawHelpText()
{
    // Setup 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1200, 0, 800, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Help panel background
    glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(400, 100);
    glVertex2f(800, 100);
    glVertex2f(800, 650);
    glVertex2f(400, 650);
    glEnd();

    // Help title bar
    glColor4f(0.8f, 0.4f, 0.2f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(400, 620);
    glVertex2f(800, 620);
    glVertex2f(800, 650);
    glVertex2f(400, 650);
    glEnd();

    // TESTO HELP LEGGIBILE
    glColor3f(1.0f, 1.0f, 0.8f);

    drawGLUTText(420, 630, GLUT_BITMAP_HELVETICA_18, "CONTROLS HELP");

    // Movement controls section
    drawGLUTText(420, 600, GLUT_BITMAP_TIMES_ROMAN_24, "MOVEMENT:");
    drawGLUTText(430, 580, GLUT_BITMAP_TIMES_ROMAN_24, "WASD: Move camera");
    drawGLUTText(430, 565, GLUT_BITMAP_TIMES_ROMAN_24, "Space/C: Up/Down");
    drawGLUTText(430, 550, GLUT_BITMAP_TIMES_ROMAN_24, "Shift: Fast movement");
    drawGLUTText(430, 535, GLUT_BITMAP_TIMES_ROMAN_24, "Ctrl: Slow movement");

    // Simulation controls
    drawGLUTText(420, 510, GLUT_BITMAP_TIMES_ROMAN_24, "SIMULATION:");
    drawGLUTText(430, 490, GLUT_BITMAP_TIMES_ROMAN_24, "P: Pause/Resume");
    drawGLUTText(430, 475, GLUT_BITMAP_TIMES_ROMAN_24, "T: Time dilation");
    drawGLUTText(430, 460, GLUT_BITMAP_TIMES_ROMAN_24, "+/-: Black hole mass");
    drawGLUTText(430, 445, GLUT_BITMAP_TIMES_ROMAN_24, "Enter: Reset");

    // View controls
    drawGLUTText(420, 420, GLUT_BITMAP_TIMES_ROMAN_24, "VIEW:");
    drawGLUTText(430, 400, GLUT_BITMAP_TIMES_ROMAN_24, "G: Toggle grid");
    drawGLUTText(430, 385, GLUT_BITMAP_TIMES_ROMAN_24, "L: Toggle lensing");
    drawGLUTText(430, 370, GLUT_BITMAP_TIMES_ROMAN_24, "I: Info panel");
    drawGLUTText(430, 355, GLUT_BITMAP_TIMES_ROMAN_24, "H: This help");

    // Special features
    drawGLUTText(420, 330, GLUT_BITMAP_TIMES_ROMAN_24, "SPECIAL:");
    drawGLUTText(430, 310, GLUT_BITMAP_TIMES_ROMAN_24, "F1-F4: Scenario presets");
    drawGLUTText(430, 295, GLUT_BITMAP_TIMES_ROMAN_24, "F: Follow ray mode");
    drawGLUTText(430, 280, GLUT_BITMAP_TIMES_ROMAN_24, "TAB: Mouse control");
    drawGLUTText(430, 265, GLUT_BITMAP_TIMES_ROMAN_24, "ESC: Exit");

    // Close instruction
    glColor3f(1.0f, 0.3f, 0.3f);
    drawGLUTText(420, 230, GLUT_BITMAP_TIMES_ROMAN_24, "Press H to close this help");

    // Restore 3D rendering
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void toggleMouseControl(GLFWwindow *window)
{
    mouseEnabled = !mouseEnabled;
    if (mouseEnabled)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.firstMouse = 1;
        printf("Mouse control ENABLED\n");
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        printf("Mouse control DISABLED - Use arrows for rotation\n");
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.movementSpeed += yoffset * 0.02f;
    if (camera.movementSpeed < 0.01f)
        camera.movementSpeed = 0.01f;
    if (camera.movementSpeed > 1.0f)
        camera.movementSpeed = 1.0f;
    printf("Movement speed: %.3f\n", camera.movementSpeed);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (!mouseEnabled)
        return;

    if (camera.firstMouse)
    {
        camera.lastX = xpos;
        camera.lastY = ypos;
        camera.firstMouse = 0;
        return;
    }

    float xoffset = xpos - camera.lastX;
    float yoffset = camera.lastY - ypos;
    camera.lastX = xpos;
    camera.lastY = ypos;

    xoffset *= camera.mouseSensitivity;
    yoffset *= camera.mouseSensitivity;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    if (camera.pitch > 89.0f)
        camera.pitch = 89.0f;
    if (camera.pitch < -89.0f)
        camera.pitch = -89.0f;

    updateCameraVectors();
}

void updatePolarCoordinates(Ray *ray, const BlackHole *bh)
{
    double dx = ray->position.x - bh->position.x;
    double dy = ray->position.y - bh->position.y;
    double dz = ray->position.z - bh->position.z;

    ray->r = sqrt(dx * dx + dy * dy + dz * dz);

    if (ray->r == 0)
    {
        ray->theta = 0.0;
        ray->phi = 0.0;
    }
    else
    {
        ray->theta = acos(dz / ray->r);
        ray->phi = atan2(dy, dx);
    }
}

void drawSphere(float cx, float cy, float cz, float radius, int slices, int stacks)
{
    for (int i = 0; i <= stacks; i++)
    {
        float lat0 = M_PI * (-0.5 + (float)(i - 1) / stacks);
        float z0 = sin(lat0);
        float zr0 = cos(lat0);

        float lat1 = M_PI * (-0.5 + (float)i / stacks);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; j++)
        {
            float lng = 2 * M_PI * (float)(j - 1) / slices;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(cx + radius * x * zr0, cy + radius * y * zr0, cz + radius * z0);

            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(cx + radius * x * zr1, cy + radius * y * zr1, cz + radius * z1);
        }
        glEnd();
    }
}

void drawBlackHole(const BlackHole *bh)
{
    // Event horizon (pure black)
    glColor3f(0.0f, 0.0f, 0.0f);
    drawSphere(bh->position.x, bh->position.y, bh->position.z, bh->schwarzschild_radius, 40, 40);

    // Photon sphere (faint ring)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.8f, 0.4f, 0.2f, 0.3f);
    drawSphere(bh->position.x, bh->position.y, bh->position.z, bh->schwarzschild_radius * 1.5f, 30, 30);
    glDisable(GL_BLEND);
}

void drawSpacetimeGrid(const BlackHole *bh)
{
    double rs = bh->schwarzschild_radius;
    glColor4f(0.2f, 0.2f, 0.8f, 0.5f);
    glLineWidth(1.0f);

    int num_lines = 40;
    double grid_min = -100.0;
    double grid_max = 100.0;
    double step = (grid_max - grid_min) / num_lines;

    // Lines parallel to X axis
    for (int i = 0; i <= num_lines; i++)
    {
        double z = grid_min + i * step;
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 150; j++)
        {
            double x = grid_min + (grid_max - grid_min) * (double)j / 150.0;
            double dx = x - bh->position.x;
            double dz = z - bh->position.z;
            double r = sqrt(dx * dx + dz * dz);
            if (r < rs * 1.5)
                continue;

            double curvature_factor = 3.0;
            double y = curvature_factor * sqrt(rs * (r - rs));
            glVertex3d(x, y, z);
        }
        glEnd();
    }

    // Lines parallel to Z axis
    for (int i = 0; i <= num_lines; i++)
    {
        double x = grid_min + i * step;
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 150; j++)
        {
            double z = grid_min + (grid_max - grid_min) * (double)j / 150.0;
            double dx = x - bh->position.x;
            double dz = z - bh->position.z;
            double r = sqrt(dx * dx + dz * dz);
            if (r < rs * 1.5)
                continue;

            double curvature_factor = 3.0;
            double y = curvature_factor * sqrt(rs * (r - rs));
            glVertex3d(x, y, z);
        }
        glEnd();
    }
}

void drawEmitter(const Emitter *emitter)
{
    glColor3f(1.0f, 0.2f, 0.2f);
    drawSphere(emitter->pos.x, emitter->pos.y, emitter->pos.z, 0.5f, 20, 20);

    glEnd();
    glDisable(GL_BLEND);
}

void drawRay(const Ray *ray)
{
    if (!ray->active)
        return;

    double length = 0.8;
    glLineWidth(2.0f);

    // Color based on distance to black hole for visual effect
    double intensity = fmin(1.0f, 50.0f / ray->r);
    glColor3f(1.0f, 1.0f - intensity * 0.5f, intensity);

    glBegin(GL_LINES);
    glVertex3f(ray->position.x, ray->position.y, ray->position.z);
    glVertex3f(ray->position.x + ray->direction.x * length,
               ray->position.y + ray->direction.y * length,
               ray->position.z + ray->direction.z * length);
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
        glVertex3f(ray->trail[idx].x, ray->trail[idx].y, ray->trail[idx].z);
    }
    glEnd();
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void updateCameraVectors()
{
    float radYaw = camera.yaw * M_PI / 180.0f;
    float radPitch = camera.pitch * M_PI / 180.0f;

    camera.frontX = cos(radYaw) * cos(radPitch);
    camera.frontY = sin(radPitch);
    camera.frontZ = sin(radYaw) * cos(radPitch);

    float frontLen = sqrt(camera.frontX * camera.frontX + camera.frontY * camera.frontY + camera.frontZ * camera.frontZ);
    camera.frontX /= frontLen;
    camera.frontY /= frontLen;
    camera.frontZ /= frontLen;

    float worldUpX = 0.0f, worldUpY = 1.0f, worldUpZ = 0.0f;

    camera.rightX = camera.frontY * worldUpZ - camera.frontZ * worldUpY;
    camera.rightY = camera.frontZ * worldUpX - camera.frontX * worldUpZ;
    camera.rightZ = camera.frontX * worldUpY - camera.frontY * worldUpX;

    float rightLen = sqrt(camera.rightX * camera.rightX + camera.rightY * camera.rightY + camera.rightZ * camera.rightZ);
    camera.rightX /= rightLen;
    camera.rightY /= rightLen;
    camera.rightZ /= rightLen;

    camera.upX = camera.rightY * camera.frontZ - camera.rightZ * camera.frontY;
    camera.upY = camera.rightZ * camera.frontX - camera.rightX * camera.frontZ;
    camera.upZ = camera.rightX * camera.frontY - camera.rightY * camera.frontX;

    if (camera.roll != 0.0f)
    {
        float radRoll = camera.roll * M_PI / 180.0f;
        float cosRoll = cos(radRoll);
        float sinRoll = sin(radRoll);

        float newUpX = camera.upX * cosRoll - camera.rightX * sinRoll;
        float newUpY = camera.upY * cosRoll - camera.rightY * sinRoll;
        float newUpZ = camera.upZ * cosRoll - camera.rightZ * sinRoll;

        float newRightX = camera.rightX * cosRoll + camera.upX * sinRoll;
        float newRightY = camera.rightY * cosRoll + camera.upY * sinRoll;
        float newRightZ = camera.rightZ * cosRoll + camera.upZ * sinRoll;

        camera.upX = newUpX;
        camera.upY = newUpY;
        camera.upZ = newUpZ;
        camera.rightX = newRightX;
        camera.rightY = newRightY;
        camera.rightZ = newRightZ;
    }

    float upLen = sqrt(camera.upX * camera.upX + camera.upY * camera.upY + camera.upZ * camera.upZ);
    camera.upX /= upLen;
    camera.upY /= upLen;
    camera.upZ /= upLen;
}

void myLookAt(float eyeX, float eyeY, float eyeZ,
              float centerX, float centerY, float centerZ,
              float upX, float upY, float upZ)
{
    float forward[3] = {centerX - eyeX, centerY - eyeY, centerZ - eyeZ};

    float len = sqrtf(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    forward[0] /= len;
    forward[1] /= len;
    forward[2] /= len;

    float right[3] = {
        forward[1] * upZ - forward[2] * upY,
        forward[2] * upX - forward[0] * upZ,
        forward[0] * upY - forward[1] * upX};

    len = sqrtf(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
    right[0] /= len;
    right[1] /= len;
    right[2] /= len;

    float up[3] = {
        right[1] * forward[2] - right[2] * forward[1],
        right[2] * forward[0] - right[0] * forward[2],
        right[0] * forward[1] - right[1] * forward[0]};

    float matrix[16] = {
        right[0], up[0], -forward[0], 0,
        right[1], up[1], -forward[1], 0,
        right[2], up[2], -forward[2], 0,
        0, 0, 0, 1};

    glMultMatrixf(matrix);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

void generateRays(const Emitter *emitter, Ray *rays, int num_rays, const BlackHole *blackhole)
{
    // Calculate direction towards black hole
    Vector3 toBlackHole;
    toBlackHole.x = blackhole->position.x - emitter->pos.x;
    toBlackHole.y = blackhole->position.y - emitter->pos.y;
    toBlackHole.z = blackhole->position.z - emitter->pos.z;

    // Normalize direction
    double length = sqrt(toBlackHole.x * toBlackHole.x + toBlackHole.y * toBlackHole.y + toBlackHole.z * toBlackHole.z);
    toBlackHole.x /= length;
    toBlackHole.y /= length;
    toBlackHole.z /= length;

    // Calculate vectors for local coordinate system of cone
    Vector3 right, up;

    // Find perpendicular vector to calculate right
    Vector3 temp = {0.0, 1.0, 0.0};
    if (fabs(toBlackHole.y) > 0.9)
    {
        temp.x = 1.0;
        temp.y = 0.0;
        temp.z = 0.0;
    }

    // Right = toBlackHole × temp
    right.x = toBlackHole.y * temp.z - toBlackHole.z * temp.y;
    right.y = toBlackHole.z * temp.x - toBlackHole.x * temp.z;
    right.z = toBlackHole.x * temp.y - toBlackHole.y * temp.x;

    // Normalize right
    length = sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
    right.x /= length;
    right.y /= length;
    right.z /= length;

    // Up = right × toBlackHole
    up.x = right.y * toBlackHole.z - right.z * toBlackHole.y;
    up.y = right.z * toBlackHole.x - right.x * toBlackHole.z;
    up.z = right.x * toBlackHole.y - right.y * toBlackHole.x;

    // Convert FOV to radians and calculate cone radius
    double fov_rad = emitter->fov * M_PI / 180.0;
    double max_angle = fov_rad / 2.0;

    for (int i = 0; i < num_rays; i++)
    {
        // Uniform sampling within cone
        double u = (double)rand() / RAND_MAX;
        double v = (double)rand() / RAND_MAX;

        // Uniform sampling in circular area (cone projection)
        double r = sqrt(u) * tan(max_angle);
        double theta = 2.0 * M_PI * v;

        // Coordinates in plane perpendicular to cone direction
        double x_local = r * cos(theta);
        double y_local = r * sin(theta);

        // Transform to world coordinate system
        Vector3 direction;
        direction.x = toBlackHole.x + x_local * right.x + y_local * up.x;
        direction.y = toBlackHole.y + x_local * right.y + y_local * up.y;
        direction.z = toBlackHole.z + x_local * right.z + y_local * up.z;

        // Normalize final direction
        length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;

        // Assign to ray
        rays[i].position = emitter->pos;
        rays[i].direction = direction;
        rays[i].trail_head = 0;
        rays[i].trail_length = 0;
        rays[i].active = 1;

        // Set color based on initial direction (for visual variety)
        rays[i].color.r = 0.8f + 0.2f * cos(theta);
        rays[i].color.g = 0.8f + 0.2f * sin(theta);
        rays[i].color.b = 0.6f + 0.4f * r / tan(max_angle);
        rays[i].color.a = 1.0f;
    }
}

void initEmitter(Emitter *emitter)
{
    emitter->pos.x = 50.0f;
    emitter->pos.y = 0.0f;
    emitter->pos.z = 0.0f;
    emitter->front.x = -1.0f;
    emitter->front.y = 0.0f;
    emitter->front.z = 0.0f;
    emitter->up.x = 0.0f;
    emitter->up.y = 0.0f;
    emitter->up.z = 1.0f;
    emitter->fov = 15.0f;
}

void initEmitter2(Emitter *emitter)
{
    emitter->pos.x = -50.0f;
    emitter->pos.y = 0.0f;
    emitter->pos.z = 0.0f;
    emitter->front.x = 1.0f;
    emitter->front.y = 0.0f;
    emitter->front.z = 0.0f;
    emitter->up.x = 0.0f;
    emitter->up.y = 0.0f;
    emitter->up.z = 1.0f;
    emitter->fov = 15.0f;
}

double distanceToCamera(const Ray *ray, const Camera *cam)
{
    double dx = ray->position.x - cam->posX;
    double dy = ray->position.y - cam->posY;
    double dz = ray->position.z - cam->posZ;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

void updateRaysLOD(Ray *rays, int num_rays, const Camera *cam, const BlackHole *bh)
{
    for (int i = 0; i < num_rays; i++)
    {
        if (!rays[i].active)
            continue;

        double dist_to_camera = distanceToCamera(&rays[i], cam);

        // Level of Detail based on distance
        int should_update = 1;
        if (dist_to_camera > LOD_DISTANCE_2)
        {
            should_update = (frame_count % 4 == 0); // Update every 4th frame
        }
        else if (dist_to_camera > LOD_DISTANCE_1)
        {
            should_update = (frame_count % 2 == 0); // Update every 2nd frame
        }
    }
}

extern Ray *current_rays;
extern int current_num_rays;

void drawScaledGLUTText(float x, float y, void *font, const char *string, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 10.0f);
    glRasterPos2f(0, 0);
    while (*string)
    {
        glutBitmapCharacter(font, *string);
        string++;
    }
    glPopMatrix();
}

void drawInfoPanel()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1200, 0, 800, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Background panel
    glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(10, 600);
    glVertex2f(400, 600);
    glVertex2f(400, 790);
    glVertex2f(10, 790);
    glEnd();

    // Edge
    glColor3f(0.3f, 0.6f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(10, 600);
    glVertex2f(400, 600);
    glVertex2f(400, 790);
    glVertex2f(10, 790);
    glEnd();

    // Title bar
    glColor4f(0.2f, 0.4f, 0.8f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(10, 770);
    glVertex2f(400, 770);
    glVertex2f(400, 790);
    glVertex2f(10, 790);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawGLUTText(20, 775, GLUT_BITMAP_TIMES_ROMAN_24, "BLACK HOLE SIMULATOR");

    glColor3f(0.9f, 0.9f, 0.9f);
    drawGLUTText(20, 750, GLUT_BITMAP_TIMES_ROMAN_24, "Schwarzschild Radius:");

    char rs_text[100];
    snprintf(rs_text, sizeof(rs_text), "Rs = %.1f", schwarzschild_radius);
    drawGLUTText(200, 750, GLUT_BITMAP_TIMES_ROMAN_24, rs_text);

    drawProgressBar(schwarzschild_radius / 5.0f, 20, 735, 200, 0.8f, 0.2f, 0.2f);

    drawGLUTText(20, 720, GLUT_BITMAP_TIMES_ROMAN_24, "Active Rays:");
    char rays_text[100];
    snprintf(rays_text, sizeof(rays_text), "%d / %d", ray_count, NUM_RAYS);
    drawGLUTText(150, 720, GLUT_BITMAP_TIMES_ROMAN_24, rays_text);

    drawProgressBar((float)ray_count / NUM_RAYS, 20, 705, 200, 0.2f, 0.8f, 0.2f);

    drawGLUTText(20, 690, GLUT_BITMAP_TIMES_ROMAN_24, "Time Dilation Factor:");
    char time_text[100];
    snprintf(time_text, sizeof(time_text), "%.1fx", time_dilation_factor);
    drawGLUTText(200, 690, GLUT_BITMAP_TIMES_ROMAN_24, time_text);

    drawProgressBar(time_dilation_factor / 2.0f, 20, 675, 200, 0.2f, 0.2f, 0.8f);

    glColor3f(paused ? 1.0f : 0.3f, paused ? 0.3f : 1.0f, 0.3f);
    drawGLUTText(20, 650, GLUT_BITMAP_TIMES_ROMAN_24, paused ? "PAUSED" : "RUNNING");

    glColor3f(showGrid ? 0.3f : 1.0f, showGrid ? 1.0f : 0.3f, 0.3f);
    drawGLUTText(120, 650, GLUT_BITMAP_TIMES_ROMAN_24, showGrid ? "GRID ON" : "GRID OFF");

    glColor3f(showLensing ? 0.3f : 1.0f, showLensing ? 1.0f : 0.3f, 0.3f);
    drawGLUTText(220, 650, GLUT_BITMAP_TIMES_ROMAN_24, showLensing ? "LENS ON" : "LENS OFF");

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawGLUTText(float x, float y, void *font, const char *string)
{
    glRasterPos2f(x, y);
    while (*string)
    {
        glutBitmapCharacter(font, *string);
        string++;
    }
}

void drawProgressBar(float value, int x, int y, int width, float r, float g, float b)
{
    if (value < 0.0f)
        value = 0.0f;
    if (value > 1.0f)
        value = 1.0f;

    glColor4f(0.2f, 0.2f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + 12);
    glVertex2f(x, y + 12);
    glEnd();

    int filled_width = (int)(value * width);
    if (filled_width > 0)
    {
        glColor4f(r, g, b, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(x + 1, y + 1);
        glVertex2f(x + filled_width - 1, y + 1);
        glVertex2f(x + filled_width - 1, y + 11);
        glVertex2f(x + 1, y + 11);
        glEnd();
    }

    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + 12);
    glVertex2f(x, y + 12);
    glEnd();
}
