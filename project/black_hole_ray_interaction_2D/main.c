#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define NUM_PHOTONS 100
#define TIME_STEP 0.3
#define MAX_TRAIL_POINTS 1500
#define STEPS_PER_FRAME 5
#define PHOTON_SPHERE_FACTOR 1.5
#define EVENT_HORIZON_FACTOR 1.0

typedef struct {
    double x, y, z;
} Vector3;

typedef struct {
    Vector3 position;
    double mass;
    double rs;             // Schwarzschild radius
} BlackHole;

typedef struct {
    double t, r, theta, phi;
    double dt_dtau, dr_dtau, dtheta_dtau, dphi_dtau;
    double energy;
    double angular_momentum;
    double x, y;
    
    Vector3 trail[MAX_TRAIL_POINTS];
    int trail_head;
    int trail_length;
    
    int active;
    int escaped;
    int captured;
} Photon;

typedef struct {
    Photon photons[NUM_PHOTONS];
    int num_active;
} PhotonArray;

void initializeBlackHole(BlackHole *bh, double mass, double center_x, double center_y);
void initializePhoton(Photon *photon, double x0, double y0, double vx, double vy, const BlackHole *bh);
void computeConservedQuantities(Photon *photon, const BlackHole *bh);
void geodesicDerivatives(const Photon *photon, const BlackHole *bh, 
                        double *dt_dtau_dot, double *dr_dtau_dot, 
                        double *dtheta_dtau_dot, double *dphi_dtau_dot);
void rungeKutta4Step(Photon *photon, const BlackHole *bh, double h);
void updateCartesianCoordinates(Photon *photon, const BlackHole *bh);
void addToTrail(Photon *photon);
int checkPhotonStatus(Photon *photon, const BlackHole *bh);

void drawBlackHole(const BlackHole *bh);
void drawPhoton(const Photon *photon);
void drawPhotonTrail(const Photon *photon);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int main(void) {
    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(1200, 900, "Black hole - rays", NULL, NULL);
    if (!window) {
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
    glOrtho(-width/2, width/2, -height/2, height/2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    BlackHole blackhole;
    initializeBlackHole(&blackhole, 1.0, 0.0, 0.0);
    
    double scale = 30.0;
    blackhole.rs *= scale;
    blackhole.mass *= scale;

    PhotonArray photons;
    photons.num_active = NUM_PHOTONS;

    for (int i = 0; i < NUM_PHOTONS; i++) {
        double y_start = -250.0 + (i * 500.0) / (NUM_PHOTONS - 1);
        double x_start = -350.0;
        double vx = 1.0;
        double vy = 0.0;
        
        initializePhoton(&photons.photons[i], x_start, y_start, vx, vy, &blackhole);
    }

    printf("Black hole mass: %.2f, Schwarzschild radius: %.2f\n", 
           blackhole.mass/scale, blackhole.rs/scale);
    printf("Photon sphere radius: %.2f\n", blackhole.rs * PHOTON_SPHERE_FACTOR/scale);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            for (int i = 0; i < NUM_PHOTONS; i++) {
                double y_start = -250.0 + (i * 500.0) / (NUM_PHOTONS - 1);
                double x_start = -350.0;
                initializePhoton(&photons.photons[i], x_start, y_start, 1.0, 0.0, &blackhole);
            }
            photons.num_active = NUM_PHOTONS;
        }

        drawBlackHole(&blackhole);

        photons.num_active = 0;
        for (int i = 0; i < NUM_PHOTONS; i++) {
            Photon *p = &photons.photons[i];
            
            if (p->active) {
                for (int step = 0; step < STEPS_PER_FRAME; step++) {
                    if (p->active) {
                        rungeKutta4Step(p, &blackhole, TIME_STEP);
                        updateCartesianCoordinates(p, &blackhole);
                        
                        if (step % 2 == 0) {
                            addToTrail(p);
                        }
                        
                        if (!checkPhotonStatus(p, &blackhole)) {
                            break;
                        }
                    }
                }
                
                if (p->active) {
                    photons.num_active++;
                }
            }
            
            drawPhotonTrail(p);
            if (p->active) {
                drawPhoton(p);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void initializeBlackHole(BlackHole *bh, double mass, double center_x, double center_y) {
    bh->mass = mass;
    bh->rs = 2.0 * mass;  // Schwarzschild radius in geometric units
    bh->position.x = center_x;
    bh->position.y = center_y;
    bh->position.z = 0.0;
}

void initializePhoton(Photon *photon, double x0, double y0, double vx, double vy, const BlackHole *bh) {
    double dx = x0 - bh->position.x;
    double dy = y0 - bh->position.y;
    
    photon->r = sqrt(dx*dx + dy*dy);
    photon->phi = atan2(dy, dx);
    photon->theta = M_PI/2.0;  // equatorial plane
    photon->t = 0.0;
    
    photon->x = x0;
    photon->y = y0;
    
    double v_mag = sqrt(vx*vx + vy*vy);
    if (v_mag > 0) {
        vx /= v_mag;
        vy /= v_mag;
    }
    
    double r = photon->r;
    double cos_phi = cos(photon->phi);
    double sin_phi = sin(photon->phi);
    
    double vr = vx * cos_phi + vy * sin_phi;
    double vphi = (-vx * sin_phi + vy * cos_phi) / r;
    
    double f = 1.0 - bh->rs/r;
    
    photon->energy = sqrt(f + vr*vr/f + r*r*vphi*vphi);
    
    photon->dt_dtau = photon->energy / f;
    photon->dr_dtau = vr;
    photon->dtheta_dtau = 0.0;
    photon->dphi_dtau = vphi;
    
    computeConservedQuantities(photon, bh);
    
    photon->trail_head = 0;
    photon->trail_length = 0;
    for (int i = 0; i < MAX_TRAIL_POINTS; i++) {
        photon->trail[i] = (Vector3){x0, y0, 0.0};
    }
    
    photon->active = 1;
    photon->escaped = 0;
    photon->captured = 0;
}

void computeConservedQuantities(Photon *photon, const BlackHole *bh) {
    double r = photon->r;
    double f = 1.0 - bh->rs/r;
    
    photon->energy = f * photon->dt_dtau;
    photon->angular_momentum = r * r * photon->dphi_dtau;
}

void geodesicDerivatives(const Photon *photon, const BlackHole *bh,
                        double *dt_dtau_dot, double *dr_dtau_dot, 
                        double *dtheta_dtau_dot, double *dphi_dtau_dot) {
    
    double r = photon->r;
    double rs = bh->rs;
    
    if (r <= rs * 1.001) {
        *dt_dtau_dot = 0.0;
        *dr_dtau_dot = 0.0;
        *dtheta_dtau_dot = 0.0;
        *dphi_dtau_dot = 0.0;
        return;
    }
    
    double f = 1.0 - rs/r;
    double dt_dtau = photon->dt_dtau;
    double dr_dtau = photon->dr_dtau;
    double dphi_dtau = photon->dphi_dtau;
    
    // Christoffel symbols for Schwarzschild metric
    double Gamma_t_tr = rs / (2.0 * r * r * f);
    double Gamma_r_tt = rs * f / (2.0 * r * r);
    double Gamma_r_rr = -rs / (2.0 * r * r * f);
    double Gamma_r_phiphi = -(r - rs);
    double Gamma_phi_rphi = 1.0 / r;
    
    // Geodesic equations: d²x^μ/dτ² + Γ^μ_νρ (dx^ν/dτ)(dx^ρ/dτ) = 0
    
    *dt_dtau_dot = -2.0 * Gamma_t_tr * dt_dtau * dr_dtau;
    
    *dr_dtau_dot = -Gamma_r_tt * dt_dtau * dt_dtau 
                   - Gamma_r_rr * dr_dtau * dr_dtau
                   - Gamma_r_phiphi * dphi_dtau * dphi_dtau;
    
    *dtheta_dtau_dot = 0.0;  // Motion confined to equatorial plane
    
    *dphi_dtau_dot = -2.0 * Gamma_phi_rphi * dr_dtau * dphi_dtau;
}

void rungeKutta4Step(Photon *photon, const BlackHole *bh, double h) {
    double t0 = photon->t;
    double r0 = photon->r;
    double theta0 = photon->theta;
    double phi0 = photon->phi;
    double dt_dtau0 = photon->dt_dtau;
    double dr_dtau0 = photon->dr_dtau;
    double dtheta_dtau0 = photon->dtheta_dtau;
    double dphi_dtau0 = photon->dphi_dtau;
    
    // k1
    double k1_t = dt_dtau0;
    double k1_r = dr_dtau0;
    double k1_theta = dtheta_dtau0;
    double k1_phi = dphi_dtau0;
    double k1_dt_dtau, k1_dr_dtau, k1_dtheta_dtau, k1_dphi_dtau;
    
    geodesicDerivatives(photon, bh, &k1_dt_dtau, &k1_dr_dtau, &k1_dtheta_dtau, &k1_dphi_dtau);
    
    // k2
    Photon temp = *photon;
    temp.t = t0 + 0.5 * h * k1_t;
    temp.r = r0 + 0.5 * h * k1_r;
    temp.theta = theta0 + 0.5 * h * k1_theta;
    temp.phi = phi0 + 0.5 * h * k1_phi;
    temp.dt_dtau = dt_dtau0 + 0.5 * h * k1_dt_dtau;
    temp.dr_dtau = dr_dtau0 + 0.5 * h * k1_dr_dtau;
    temp.dtheta_dtau = dtheta_dtau0 + 0.5 * h * k1_dtheta_dtau;
    temp.dphi_dtau = dphi_dtau0 + 0.5 * h * k1_dphi_dtau;
    
    double k2_t = temp.dt_dtau;
    double k2_r = temp.dr_dtau;
    double k2_theta = temp.dtheta_dtau;
    double k2_phi = temp.dphi_dtau;
    double k2_dt_dtau, k2_dr_dtau, k2_dtheta_dtau, k2_dphi_dtau;
    
    geodesicDerivatives(&temp, bh, &k2_dt_dtau, &k2_dr_dtau, &k2_dtheta_dtau, &k2_dphi_dtau);
    
    // k3
    temp.t = t0 + 0.5 * h * k2_t;
    temp.r = r0 + 0.5 * h * k2_r;
    temp.theta = theta0 + 0.5 * h * k2_theta;
    temp.phi = phi0 + 0.5 * h * k2_phi;
    temp.dt_dtau = dt_dtau0 + 0.5 * h * k2_dt_dtau;
    temp.dr_dtau = dr_dtau0 + 0.5 * h * k2_dr_dtau;
    temp.dtheta_dtau = dtheta_dtau0 + 0.5 * h * k2_dtheta_dtau;
    temp.dphi_dtau = dphi_dtau0 + 0.5 * h * k2_dphi_dtau;
    
    double k3_t = temp.dt_dtau;
    double k3_r = temp.dr_dtau;
    double k3_theta = temp.dtheta_dtau;
    double k3_phi = temp.dphi_dtau;
    double k3_dt_dtau, k3_dr_dtau, k3_dtheta_dtau, k3_dphi_dtau;
    
    geodesicDerivatives(&temp, bh, &k3_dt_dtau, &k3_dr_dtau, &k3_dtheta_dtau, &k3_dphi_dtau);
    
    // k4
    temp.t = t0 + h * k3_t;
    temp.r = r0 + h * k3_r;
    temp.theta = theta0 + h * k3_theta;
    temp.phi = phi0 + h * k3_phi;
    temp.dt_dtau = dt_dtau0 + h * k3_dt_dtau;
    temp.dr_dtau = dr_dtau0 + h * k3_dr_dtau;
    temp.dtheta_dtau = dtheta_dtau0 + h * k3_dtheta_dtau;
    temp.dphi_dtau = dphi_dtau0 + h * k3_dphi_dtau;
    
    double k4_t = temp.dt_dtau;
    double k4_r = temp.dr_dtau;
    double k4_theta = temp.dtheta_dtau;
    double k4_phi = temp.dphi_dtau;
    double k4_dt_dtau, k4_dr_dtau, k4_dtheta_dtau, k4_dphi_dtau;
    
    geodesicDerivatives(&temp, bh, &k4_dt_dtau, &k4_dr_dtau, &k4_dtheta_dtau, &k4_dphi_dtau);
    
    photon->t += h * (k1_t + 2*k2_t + 2*k3_t + k4_t) / 6.0;
    photon->r += h * (k1_r + 2*k2_r + 2*k3_r + k4_r) / 6.0;
    photon->theta += h * (k1_theta + 2*k2_theta + 2*k3_theta + k4_theta) / 6.0;
    photon->phi += h * (k1_phi + 2*k2_phi + 2*k3_phi + k4_phi) / 6.0;
    photon->dt_dtau += h * (k1_dt_dtau + 2*k2_dt_dtau + 2*k3_dt_dtau + k4_dt_dtau) / 6.0;
    photon->dr_dtau += h * (k1_dr_dtau + 2*k2_dr_dtau + 2*k3_dr_dtau + k4_dr_dtau) / 6.0;
    photon->dtheta_dtau += h * (k1_dtheta_dtau + 2*k2_dtheta_dtau + 2*k3_dtheta_dtau + k4_dtheta_dtau) / 6.0;
    photon->dphi_dtau += h * (k1_dphi_dtau + 2*k2_dphi_dtau + 2*k3_dphi_dtau + k4_dphi_dtau) / 6.0;
}

void updateCartesianCoordinates(Photon *photon, const BlackHole *bh) {
    photon->x = bh->position.x + photon->r * cos(photon->phi);
    photon->y = bh->position.y + photon->r * sin(photon->phi);
}

void addToTrail(Photon *photon) {
    photon->trail[photon->trail_head] = (Vector3){photon->x, photon->y, 0.0};
    photon->trail_head = (photon->trail_head + 1) % MAX_TRAIL_POINTS;
    if (photon->trail_length < MAX_TRAIL_POINTS) {
        photon->trail_length++;
    }
}

int checkPhotonStatus(Photon *photon, const BlackHole *bh) {
    if (photon->r <= bh->rs * 1.01) {
        photon->active = 0;
        photon->captured = 1;
        return 0;
    }
    
    if (photon->r > bh->rs * 50.0) {
        photon->active = 0;
        photon->escaped = 1;
        return 0;
    }
    
    return 1;
}

void drawBlackHole(const BlackHole *bh) {
    double rs = bh->rs;
    
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 100; i++) {
        float angle = 2.0f * M_PI * i / 100;
        float x = cos(angle);
        float y = sin(angle);
        
        float temp_inner = 1.0f;
        glColor4f(temp_inner, temp_inner * 0.7f, temp_inner * 0.3f, 0.8f);
        glVertex2f(bh->position.x + x * rs * 3.0f, bh->position.y + y * rs * 3.0f);
        
        float temp_outer = 0.3f;
        glColor4f(temp_outer, temp_outer * 0.8f, temp_outer * 1.2f, 0.4f);
        glVertex2f(bh->position.x + x * rs * 6.0f, bh->position.y + y * rs * 6.0f);
    }
    glEnd();
    
    glColor4f(0.7f, 0.7f, 1.0f, 0.6f);
    glLineStipple(2, 0x5555);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++) {
        float angle = 2.0f * M_PI * i / 100;
        glVertex2f(bh->position.x + cos(angle) * rs * PHOTON_SPHERE_FACTOR,
                  bh->position.y + sin(angle) * rs * PHOTON_SPHERE_FACTOR);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(bh->position.x, bh->position.y);
    for (int i = 0; i <= 100; i++) {
        float angle = 2.0f * M_PI * i / 100;
        glVertex2f(bh->position.x + cos(angle) * rs,
                  bh->position.y + sin(angle) * rs);
    }
    glEnd();
    
    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++) {
        float angle = 2.0f * M_PI * i / 100;
        glVertex2f(bh->position.x + cos(angle) * rs,
                  bh->position.y + sin(angle) * rs);
    }
    glEnd();
}

void drawPhoton(const Photon *photon) {
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(photon->x, photon->y);
    for (int i = 0; i <= 8; i++) {
        float angle = 2.0f * M_PI * i / 8;
        glVertex2f(photon->x + cos(angle) * 3.0f, photon->y + sin(angle) * 3.0f);
    }
    glEnd();
}

void drawPhotonTrail(const Photon *photon) {
    if (photon->trail_length < 2) return;
    
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < photon->trail_length; i++) {
        int idx = (photon->trail_head - photon->trail_length + i + MAX_TRAIL_POINTS) % MAX_TRAIL_POINTS;
        float alpha = (float)i / photon->trail_length;
        
        if (photon->captured) {
            glColor4f(1.0f, 0.2f, 0.2f, alpha * 0.8f);  // Red for captured photons
        } else if (photon->escaped) {
            glColor4f(0.2f, 1.0f, 0.2f, alpha * 0.8f);  // Green for escaped photons
        } else {
            glColor4f(1.0f, 1.0f, 0.0f, alpha * 0.8f);  // Yellow for active photons
        }
        
        glVertex2f(photon->trail[idx].x, photon->trail[idx].y);
    }
    glEnd();
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-width/2, width/2, -height/2, height/2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}