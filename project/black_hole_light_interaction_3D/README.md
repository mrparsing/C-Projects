# Black Hole Simulation with Ray Tracing

![Demo1](../assets/black%20hole%203D.gif)
![Demo2](../assets/black%20hole%203D%202.gif)

This program simulates the gravitational interactions between three celestial bodies: two stars orbiting a central supermassive black hole. The rendering is performed using ray tracing techniques that incorporate the effects of general relativity, specifically the bending of light rays due to gravity as described by the Schwarzschild metric. The simulation visualizes the spacetime curvature around the black hole, including gravitational lensing, an accretion disk, and the motion of the stars under Newtonian gravity (approximated for the N-body simulation).

The program uses OpenGL for rendering, GLFW for window and input management, and custom shaders for ray tracing and grid visualization. It runs in real-time, allowing interactive camera control to orbit, pan, and zoom around the scene.

### Key Features
- **Physics Simulation**: Newtonian N-body gravity for celestial body motion.
- **Ray Tracing**: Simulates light paths bent by the black hole's gravity using geodesic equations in the Schwarzschild metric.
- **Visualizations**: 
  - Deformed spacetime grid showing curvature.
  - Accretion disk around the black hole.
  - Stars rendered with basic shading.
  - Background stars for escaped rays.
- **Interactivity**: Camera controls, pause/resume physics, toggle grid.

### Requirements
- **Libraries**: GLFW (for windowing and input), OpenGL 3.3+ (core profile).
- **Build Tools**: A C compiler (e.g., GCC or Clang) that supports C99.
- **Platform**: Tested on macOS (with OpenGL deprecation silenced), but should work on Windows/Linux with minor adjustments to build commands.
- No external dependencies beyond GLFW and OpenGL.

### Controls
- **Left Mouse + Drag**: Orbit the camera around the target point.
- **Middle Mouse + Drag**: Pan the camera horizontally/vertically.
- **Mouse Wheel**: Zoom in/out (clamped between min and max radius).
- **R**: Reset camera to initial position.
- **P**: Pause/resume the physics simulation.
- **G**: Toggle visibility of the spacetime grid.
- **ESC**: Exit the application.

### Code Explanation

The code is structured as a single-file C program for simplicity. It includes definitions, utility functions, camera handling, physics simulation, grid generation, shader utilities, rendering engine, callbacks, and the main loop.

#### 1. Includes and Definitions
- **Includes**: Standard C libraries (stdio, stdlib, math, etc.), OpenGL/GLFW headers.
- **Constants**:
  - Physical: Speed of light (`SPEED_OF_LIGHT`), gravitational constant (`GRAVITATIONAL_CONSTANT`), Schwarzschild radius for the black hole (`BLACK_HOLE_SCHWARZSCHILD_RADIUS` ≈ 1.269e10 m, roughly for a supermassive black hole like Sagittarius A* with mass ~4 million solar masses).
  - Simulation: Ray integration step (`RAY_INTEGRATION_STEP`), escape radius for rays (`RAY_ESCAPE_RADIUS`).
  - The Schwarzschild radius \( $r_s = \frac{2GM}{c^2}$ \), where \(G\) is the gravitational constant, \(M\) is mass, and \(c\) is speed of light, defines the event horizon of a non-rotating black hole.
- **Structures**:
  - `vector3_t`, `vector4_t`, `matrix4_t`: For 3D/4D vectors and 4x4 matrices (column-major for OpenGL compatibility).
  - `camera_t`: Manages orbital camera state (radius, azimuth, elevation, etc.).
  - `celestial_body_t`: Position/radius (in vec4), color, mass, velocity.
  - `renderer_engine_t`: Holds GLFW window, VAOs, shaders, textures, etc.
- **Global State**:
  - Initial camera setup.
  - Array of celestial bodies: Two stars (blue and red, solar mass ~1.989e30 kg, initial positions and velocities for orbiting), central black hole (mass ~8.54e36 kg, fixed at origin).
  - Renderer engine instance.

#### 2. Math Utility Functions
- Basic vector operations: length, normalize, cross product, subtract/add/scale.
- Matrix functions: Identity, perspective projection, look-at view matrix, multiplication.
- These are essential for camera transformations and rendering. Physically, the perspective projection simulates how light rays converge to a focal point (like a camera lens), while the view matrix positions the observer in 3D space.

#### 3. Camera Functions
- `camera_get_position`: Computes camera world position from spherical coordinates (azimuth, elevation, radius around target).
- `camera_process_mouse_move`: Handles orbiting (azimuth/elevation) and panning (shift target using right/up vectors).
- `camera_process_scroll`: Zooms by adjusting radius, clamped to avoid getting too close/far (min 1e10 m, max 25e10 m).
- `camera_reset`: Restores initial state.
- Physically, this orbital camera allows viewing gravitational effects from different angles, similar to observing a black hole system from afar.

#### 4. Physics Simulation
- `simulation_update_physics(delta_time)`:
  - If paused, skip.
  - For each pair of bodies, compute gravitational force \($F=\frac{G m_1 m_2}{r^2}$\), acceleration \($a = \frac{F}{m}$\), update velocity \($v \pm a \Delta t$\).
  - Avoid self-interaction and collisions (check distance > sum of radii).
  - Update positions \($p \pm v \Delta t$\).
- This is a Newtonian N-body simulation (Euler integration). In reality, near black holes, general relativity (GR) effects like frame-dragging would apply, but for simplicity, Newtonian gravity suffices for body motion. The black hole is fixed (high mass), and stars orbit with initial velocities ~5.34e7 m/s (realistic for galactic scales).
- Time is scaled up (`delta_time * 500`) to make motion visible in real-time.

#### 5. Spacetime Grid Rendering
- `grid_generate_mesh`:
  - Creates a 50x50 grid (-25e10 to +25e10 m, spacing 1e10 m).
  - For each point, compute height deformation due to gravitational potential.
  - Uses Flamm's paraboloid approximation for curvature: \($\delta y \approx \sqrt{8 r_s (r - r_s)}$\), where \($r_s$\) is Schwarzschild radius.
  - Scaled for visual effect (planets x500).
  - Generates vertices/indices for line drawing.
- `grid_render`: Draws lines using a simple shader, with view-projection matrix.
- Physically, this visualizes spacetime curvature in the Schwarzschild metric, where gravity warps space like a rubber sheet. The grid dips near massive objects, illustrating how paths (geodesics) curve.

#### 6. OpenGL Shader Utilities
- `utility_compile_shader` and `utility_create_shader_program`: Compile and link shaders, with error checking.
- Shaders are defined at the end: quad (for fullscreen), grid (simple lines), raytracer (complex fragment shader).

#### 7. Renderer Engine Functions
- `engine_init_fullscreen_quad`: VAO for a fullscreen quad (used for ray tracing and texture display).
- `engine_init_render_texture`: Creates a low-res texture (window size /7) for ray tracing output.
- `engine_render_raytraced_scene_to_texture`:
  - Binds framebuffer with texture.
  - Sets uniforms: Camera vectors, FOV, aspect, moving flag, disk radii, celestial bodies.
  - Draws quad with raytracer shader.
- `engine_render_texture_to_screen`: Draws the texture fullscreen with blending.
- The ray tracing happens in the fragment shader (GPU-accelerated).

#### 8. Raytracer Shader (Fragment Shader)
This is the core of the GR simulation. It traces rays backward from the camera through spacetime.

- **Uniforms**: Camera position/directions, FOV, disk radii, objects.
- **Ray Structure**: Position (cartesian/spherical), derivatives (dr, dtheta, dphi), conserved quantities E (energy), L (angular momentum).
- **initRay**: Converts cartesian pos/dir to spherical coords. Computes E and L from the Schwarzschild metric, where geodesics conserve these due to symmetries (time-translation and rotation).
- **intercept**: Checks if ray inside event horizon (\($r \leq r_s$\)).
- **interceptObject**: Checks sphere intersection for stars/black hole.
- **geodesicRHS**: Computes right-hand side for geodesic equations in Schwarzschild coords:
  - Metric: \($ds^2 = -(1 - \frac{r_s}{r}) dt^2 + (1 - \frac{r_s}{r})^{-1} dr^2 + r^2 d\theta^2 + r^2 \sin^2\theta d\phi^2$\).
  - Equations derived from Euler-Lagrange for null geodesics (light rays).
- **rk4Step**: 1st-order Runge-Kutta (actually Euler-like here) to integrate geodesics with step \($d\lambda$\) (affine parameter).
- **crossesEquatorialPlane**: Detects accretion disk hit (thin disk in xy-plane, inner/outer radii 2.2-5.2 \($r_s$\)).
- **Main Loop**:
  - Initialize ray from pixel.
  - Integrate up to 25k-26k steps (fewer if moving for speed).
  - Adaptive step: Smaller near black hole (\($\propto r / (20 r_s)$\)).
  - Check hits: Black hole (black), disk (colored with temperature gradient and spiral pattern), object (shaded with Phong model), or escape (star field).
- Physically: Light follows null geodesics in curved spacetime. Near black holes, photons can orbit (photon sphere at 1.5 \($r_s$\)), lens, or fall in. The accretion disk is modeled as hot plasma emitting light, with color from blackbody (hot inner, cool outer). Background stars simulate Milky Way-like field.

#### 9. GLFW Callbacks
- Mouse/button/scroll/key handlers call camera/physics functions.
- Framebuffer resize updates texture resolution.

#### 10. Application Lifecycle
- `engine_initialize`: Sets up GLFW/OpenGL, shaders, textures, callbacks.
- `main`: Resets camera, initializes engine, generates grid.
- Loop: Update physics/grid if not paused, compute view/proj matrices, render grid/raytrace/texture, swap buffers.
- `engine_cleanup`: Frees resources.

### Performance Considerations
The program is optimized for real-time performance on consumer hardware, but ray tracing geodesics is computationally intensive (thousands of steps per pixel).

- **Window Size**: The default window is small (500x300 pixels) to reduce the number of pixels processed, improving frame rates. Larger windows increase GPU load proportionally.
- **Render Resolution**: The ray-traced texture is downsampled (`window_width / 7`, `window_height / 7`), resulting in ~71x42 pixels initially. This low resolution drastically boosts performance by reducing fragment shader invocations (ray tracing cost scales with pixels). The texture is then upscaled to fullscreen, which may look pixelated but allows interactive rates.
- **To Improve Resolution**: Increase the division factor in `engine_initialize` and `callback_framebuffer_size` (e.g., divide by 1 for full resolution). However, this may drop FPS below 30 on mid-range GPUs—test incrementally. For even better performance, reduce integration steps (e.g., 10k when moving) or use a more efficient integrator.
- **Other Optimizations**: Adaptive stepping focuses computation near the black hole; GPU parallelism handles per-pixel tracing efficiently. On high-end hardware, full HD ray tracing is feasible but may require lowering steps or pausing motion.