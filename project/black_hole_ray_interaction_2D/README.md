# Black Hole simulation
Black holes have the ability to curve light around them. To simulate this I need to create a ray tracing engine.
Let's start with a 2D model. First, we initialize OpenGL.
```c
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <stdio.h>

int main(void) {

	if (!glfwInit()) {
		printf("Error initialization GLFW\n");
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(800, 600, "Black Hole", NULL, NULL);
	if (!window) {
		printf("Error window creation\n");
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	
	// MAIN LOOP
	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // BLACK
		glClear(GL_COLOR_BUFFER_BIT);
	
		glfwSwapBuffers(window);
	  
		glfwPollEvents();
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
	}
```
First, I create two structures: one for the black hole and one for the light rays.
The black hole has a position vector, a mass, and an event horizon.
The event horizon is the distance from the black hole beyond which not even light can escape. We can calculate it using this formula
$$event\_horizon = \frac{2 * G * M}{c^2}$$
```c
typedef struct {
	double x;
	double y;
	double z;
} Vector3;

typedef struct {
	Vector3 position;
	double mass;
	double event_horizon;
} BlackHole;
```
Now, because OpenGL doesn't have a default circle function, we have to create our own.
```c
void drawBlackHole(const BlackHole *bh)
{
	glColor3f(1.0, 0.0, 0.0);	
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(bh->position.x, bh->position.y);
	for (int i = 0; i <= 100; i++)
	{
		float angle = 2.0f * M_PI * i / 100;	
		float x = cos(angle) * bh->event_horizon + bh->position.x;
		float y = sin(angle) * bh->event_horizon + bh->position.y;
		
		glVertex2f(x, y);
	}
	glEnd();

}
```
After the creation of the window and the makeContextCurrent, we need to setup viewport and 2D ortographics 
```c
int width, height;
glfwGetFramebufferSize(window, &width, &height);
glViewport(0, 0, width, height);

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
glOrtho(0, width, 0, height, -1, 1);

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();

BlackHole blackhole = {
    .position = {width - 400, height / 2.0, 0.0},
    .mass = 1.0e30,
    .event_horizon = 50.0
};
```
After this we can call drawBlackHole inside the main loop. An this should be the result:
![[Screenshot 2025-08-11 alle 13.38.23.png]]
Alright, now let’s create the rays. These will have an x and y coordinate and a direction. Let’s make a function to draw a ray and a step function to simulate the ray’s movement, and finally put everything together in the main function.
```c
typedef struct {
	double x;
	double y;	
	Vector3 direction;
} Ray;

void drawRay(const Ray *ray)
{
	double length = 100.0;
	double endX = ray->x + ray->direction.x * length;
	double endY = ray->y + ray->direction.y * length;
	
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex2f(ray->x, ray->y);
	glVertex2f(endX, endY);
	glEnd();
}

void step(Ray *ray)
{
	ray->x += ray->direction.x * C_SPEED;
	ray->y += ray->direction.y * C_SPEED;
}

// MAIN
Ray ray = {
	.x = 100.0,
	.y = height / 2.0,
	.direction = {1.0, 0.0, 0.0}
};
```
Instead of drawing a single ray, we can drawing 15 rays, calling drawRay() for each one.
```c
typedef struct
{
	Ray ray[15];
} ArrayRay;
```
At this point, the rays pass through the black hole without interacting with it.
So now we will implement physics to alter the rays’ paths.

We will use polar coordinates. The polar coordinates are centered at the black hole’s center.
**R** is the distance from the black hole’s center to the ray, and **φ** is the angle from the x-axis. Since our black hole is at position (0, 0), **R** can be calculated using the hypotenuse function, and **φ** can be calculated using the atan function.

Before moving forward, we need to talk about the geodesic. A geodesic is the shortest line — or more precisely, the most direct path — between two points on a surface or in a curved space. On a flat surface, like a sheet of paper, the geodesic coincides with a straight line, while on curved surfaces, like a sphere, the geodesic becomes a great circle arc, which is the shortest path on the surface itself.
In modern physics, particularly in the theory of general relativity, space is not simply three-dimensional but combined with time to form four-dimensional spacetime. Bodies with mass and energy curve this spacetime, and the curvature determines how objects move within it. In this context, a geodesic is the trajectory followed by a freely falling body — that is, the “natural” path that minimizes distance in curved spacetime.

Our goal now is to have our rays follow a geodesic. To find a special type of path in spacetime, called a null geodesic, we rely on Einstein’s field equation. This equation connects the presence of mass and energy to the curvature of spacetime itself. However, it doesn’t directly give us the exact path to follow. To find the actual trajectories, we need to solve the equation under specific conditions—a task known to be one of the toughest in theoretical physics.
Back in 1915, Karl Schwarzschild tackled this problem by considering what would happen if the universe were empty except for a single, static, perfectly spherical mass. In this scenario, the mass-energy terms on the right-hand side of the equation vanish, becoming zero. By solving it under these conditions, he derived what’s now called the Schwarzschild metric, which describes the curvature of spacetime around a non-rotating black hole. This is the metric we’ll use in our simulation.
$$G_{\mu\nu} + \Lambda g_{\mu v} = \frac{8\pi G}{c^4} T_{\mu\nu}$$
- $G_{\mu\nu}$ is the Einstein tensor, representing the curvature of spacetime.
- $\Lambda$ is the cosmological constant, related to vacuum energy.
- $g_{\mu\nu}$ is the metric tensor, defining the geometry of spacetime.
- $G$ is Newton’s gravitational constant.
- $c$ is the speed of light in vacuum.
- $T_{\mu\nu}$ is the energy-momentum tensor, describing the distribution of mass and energy.
### **Physical meaning of the equation**
This equation basically says: **“The curvature of spacetime is caused by the presence of mass and energy.”** However, it doesn’t directly give us the trajectories to follow. To determine the exact path of light or an object, we need to solve this equation under specific conditions.
### **Schwarzschild’s solution**
Back in 1915, Karl Schwarzschild tackled this problem in a very simple but important case: he imagined a universe empty except for a single, static, perfectly spherical, non-rotating mass. In this special case, the energy-momentum tensor $T_{\mu\nu}$ becomes zero:
$$T_{\mu\nu} = 0$$
so the equation reduces to:
$$G_{\mu\nu} + \Lambda g_{\mu\nu} = 0$$
Schwarzschild solved this simplified version and found the metric (the function $g_{\mu\nu}$) describing the curvature of spacetime around this mass. This metric, called the **Schwarzschild metric**, is the foundation for describing the gravitational field of a non-rotating black hole.
### **What does the Schwarzschild metric mean?**
The metric gives a formula to calculate distances and time intervals in curved spacetime. In spherical coordinates $(t, r, \theta, \phi)$, the Schwarzschild metric looks like:
$$ds^2 = -\left(1 - \frac{2GM}{c^2 r}\right)c^2 dt^2 + \left(1 - \frac{2GM}{c^2 r}\right)^{-1} dr^2 + r^2 d\theta^2 + r^2 \sin^2 \theta \, d\phi^2$$
where:
- M is the mass of the body,
- r is the radial distance from the center,
- t is the time measured by a distant observer.
### **How do we use all this to trace rays?**
From this metric we can derive the **Christoffel symbols**, which allow us to write the **geodesic equation**:
$$\frac{d^2 x^\mu}{d \lambda^2} + \Gamma^\mu_{\alpha \beta} \frac{d x^\alpha}{d \lambda} \frac{d x^\beta}{d \lambda} = 0$$
This equation describes the “straightest possible” path in curved spacetime, that is, the trajectory that light (or a particle) follows. Here, \lambda is an affine parameter measuring progress along the trajectory.
Solving this equation for light rays (null geodesics, where $ds^2=0$) lets us understand how light bends around the black hole, allowing us to accurately simulate their behavior.
```c
void geodesic(Ray *ray, const BlackHole *bh, double dt)
{
	// Schwarzschild metric terms
	double rs = bh->schwarzschild_radius;
	double r = ray->r;
	
	// Avoid division by zero and inside event horizon
	if (r <= rs)
		return;

	double A = 1.0 - rs / r;	
	double dA_dr = rs / (r * r);
	
	double d2r = r * A * ray->dphi * ray->dphi - (dA_dr / (2.0 * A)) * ray->dr * ray->dr - (C_SPEED * C_SPEED * dA_dr) / (2.0 * A);
	double d2phi = -2.0 * ray->dr * ray->dphi / r;
	
	ray->dr += d2r * dt;
	ray->dphi += d2phi * dt;
	
	ray->r += ray->dr * dt;
	ray->phi += ray->dphi * dt;
	
	double vr = ray->dr;
	double vphi = ray->dphi;
	
	// Convert to Cartesian components
	ray->direction.x = vr * cos(ray->phi) - r * vphi * sin(ray->phi);
	ray->direction.y = vr * sin(ray->phi) + r * vphi * cos(ray->phi);
	
	// Normalize direction vector
	double len = hypot(ray->direction.x, ray->direction.y);
	if (len > 0)
	{
		ray->direction.x /= len;
		ray->direction.y /= len;
	}
}
```
Acceleration calculation from geodesic equations + numerical integration (Euler)

The main problem with the **Euler method** is that:
- **It’s not very accurate** – the local error is proportional to \Delta t^2, and the global error is proportional to \Delta t. This means that unless you use extremely small steps, the trajectory will quickly “drift” away from the true solution. 
- **It’s unstable** – for “stiff” systems or those with strong curvature (like near a black hole), it can blow up numerically: orbits deform and rays end up where they shouldn’t.
- **It doesn’t conserve physical quantities** – in theory, a geodesic should conserve energy and angular momentum (due to symmetries of the metric), but Euler introduces errors that make these constants of motion drift, causing artificial spirals or distorted trajectories. 
- **Error accumulation** – each step introduces a small error, and over time these errors add up, degrading the simulation.
For this reason, in astrophysics and numerical relativity, **Runge–Kutta 4 (RK4)** is almost always used.
```c
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
```
And the final result should be this

![Demo](../assets/black%20hole.gif)