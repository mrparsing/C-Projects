# C Projects — Learn by Building

I created this repository as a collection of C projects to help you practice, experiment, and master the C language. Each focuses on a specific aspect, such as graphics, algorithms, or memory management, and I've ranked them by difficulty on a scale of 1 to 5 stars.

This repository is still under development, so I'll be adding new projects from time to time. If you'd like to contribute, suggest ideas, or report any bugs, feel free to file a bug report or submit a pull request—I'm open to anything.

---

## Projects

### ⭐☆☆☆☆ (1 / 5)

- **[Caesar Chipher](./project/caesar%20chiper/)** — Implementation of the classical Caesar shift cipher.
- **[Vigenère Cipher](./project/vigenere/)** — A straightforward command‑line tool that encrypts a message with a **Vigenère‑like cipher**.
- **[File Obfuscator](./project/file_obfuscator/)** — A C utility that applies a **bitwise NOT** or a **byte-wise XOR** to the contents of a file and writes the transformed bytes to an output file.  
- **[Happy Number Checker](./project/happy_numbers/)** — A tiny command‑line utility that determines whether a given integer is a **happy number**.
- **[Sieve of Eratosthenes](./project/sieve_of_eratosthenes/)** — A simple Sieve of Eratosthenes.
- **[Pascal's Triangle](./project/pascal_triangle/)** — A C program that requests an integer `row ≥ 1` from the user and prints **Pascal’s Triangle** up to that row.
- **[Credit Card Validator](./project/credit_card_validator/)** — A command‑line utility that checks whether a numeric string passes the **Luhn checksum**.
- **[Collatz Conjecture](./project/collatz_conjecture/)** — Given an integer **n**, this program repeatedly applies the Collatz rules.
- **[FizzBuzz](./project/fizzbuzz/)** — A program that prints numbers from **1 to *n***, replacing multiples of 3 with **"Fizz"**, multiples of 5 with **"Buzz"**, and multiples of both with **"FizzBuzz"**.
- **[TCP Client/Server](./project/tcp_client_server/)** — A simple implementation of a TCP client and server.
- **[Temperature Converter](./project/temperature_converter/)** — A simple command-line C program to convert temperatures between Celsius (C), Fahrenheit (F), and Kelvin (K).
- **[Palindrome checker](./project/palindrome_checker/)** — A program that checks whether a string passed via the command line is a **palindrome**
- **[Number Base Converter](./project/base_converter/)** — A command-line tool written in C to convert numbers between Decimal, Binary, and Hexadecimal systems.
- **[Word and Character Counter](./project/word_counter/)** — A program that reads a text file and counts the number of **words** and the number of **characters**.
- **[Binary Search](./project/binary_search/)** — A binary search implementation.

---

### ⭐⭐☆☆☆ (2 / 5)

- **[Stack and Queue](./project/queue_stack/)** — Implementations of the data structures **stack** and **queue**.
- **[Number‑to‑Words Converter](./project/number_names/)** — A program that converts an integer in the range **0 – 9999** into its English words representation.
- **[libcurl GET Client](./project/curl/)** — A minimal example program that performs an HTTP **GET** request using **libcurl**.  
- **[Mandelbrot Set](./project/mandelbrot_set/)** — Render the Mandelbrot set to an SDL window.  
- **[Pong game](./project/pong_game/)** — A simple two-player Pong clone in C using **SDL2** and **SDL\_ttf**, with a basic victory screen.  
- **[N-Queens Solver](./project/8_queens/)** — Place **N queens** on an N×N chessboard so none attack each other (no same row, column, or diagonal).
- **[Tower of Hanoi](./project/hanoi/)** — Classic recursive solution to the **Tower of Hanoi**.
- **[Tic Tac Toe](./project/tictactoe/)** — A classic Tic Tac Toe game where you play against an AI opponent that uses the minimax algorithm.
- **[Magic Square](./project/magic_square/)** — This program builds a **magic square of size N×N** using the classic *Siamese method*.
- **[Monte Carlo π Estimator](./project/pi_estimation/)** — This program estimates **π** by randomly sampling points in a square and counting how many fall inside an inscribed circle.
- **[Barnsley Fern](./project/barnsley_fern/)** — A program that renders the classic **Barnsley Fern fractal**.
- **[Game of life](./project/game_of_life/)** — Implementation of **Conway’s Game of Life**.
- **[Flappy Bird](./project/flappy_bird/)** — A remake of the iconic Flappy Bird game built with SDL2.

---

### ⭐⭐⭐☆☆ (3 / 5)

- **[Ball gravity simulation](./project/ball_gravity_simulation/)** — An **interactive particle sandbox**: balls fall, collide with each other (elastic-ish with restitution), bounce on walls/floor, and resolve penetration against obstacles.
- **[Linux Keylogger](./project/linux_keylogger/)** — A minimal C program that reads **raw key-press events** from the Linux *evdev* interface.
- **[Raytracer 2D (SDL2)](./project/raytracing/)** — A simple 2D raytracer that visualizes light interacting with a circle.
- **[Rotating Cube](./project/cube/)** — A program that renders a rotating 3-D wireframe cube using **SDL2**.
- **[ASCII Rotating Cube](./project/terminal_cube/)** — Another 3D cube rotating but in the terminal.  
- **[Two-Body Orbit Simulation](./project/orbiting_planets/)** — A program that simulates a two-body gravitational interaction. A small blue planet orbits a larger yellow body, with a trail showing its past positions.
- **[Tiny Regex/Grep](./project/regex/)** — A super–small regular‑expression engine (supports `.` `*` `^` `$`) and a mini **grep** that scans stdin or files line‑by‑line.
- **[Plinko](./project/plinko/)** — A program that simulates a Plinko board game using SDL2.
- **[K-Means + Visualization](./project/kmeans/)** — This project implements the K-means clustering algorithm and visualizes the results.
- **[Black Hole - 2D - Ray Simulation](./project/black_hole_ray_interaction_2D/)** — A program that traces light rays’ paths around a black hole using the geodesic equations of the Schwarzschild metric. It visually demonstrates how spacetime curvature bends ray trajectories with dynamic animations and colorful trails.

---

### ⭐⭐⭐⭐☆ (4 / 5)

- **[Neural Network from scratch](./project/neural_network/)** — A minimal multi-layer perceptron that learns the XOR function using **stochastic gradient descent**, **binary cross-entropy**, and **sigmoid activations**.
- **[Double Pendulum](./project/double_pendulum/)** — A program that simulates a double pendulum.
- **[Three-Body Problem](./project/3_body_problem/)** — A leap-frog integrator simulating the famous three-body figure-8 orbit and drawing trails.
- **[A* visualization](./project/a_star_visualization/)** — A program to visualize the A* pathfinding algorithm.
- **[Graph Visualizer with Bellman-Ford & Dijkstra Algorithm](./project/graph_visualizer/)** — An interactive graph visualizer that implements the Bellman-Ford algorithm to find the shortest path between nodes.
- **[MD5 Implementation](./project/md5/)** — Implementation of the MD5 algorithm.  
- **[RSA Encryption and Decryption with GMP](./project/rsa/)** — RSA implementation using C and the GMP library for handling big numbers.  
- **[Sudoku Generator & Solver](./project/sudoku/)** — Generates a **fully‑valid Sudoku** and solves it with backtracking.
- **[Shell](./project/shell/)** — A minimal command-line shell-like emulator written in C using the ncurses library.
- **[Black Hole Photon Simulation - 2D](./project/black_hole_ray_interaction_2D/)** — This project simulates photon trajectories around a Schwarzschild black hole using OpenGL and GLFW, visualizing their paths, event horizon, and photon sphere with a Runge-Kutta 4th-order integration for geodesic equations.
- **[Black Hole - 2D](./project/black_hole_light_interaction_2D/)** — An OpenGL simulation of a black hole with a gravitational lensing effect: the scene (starfield background + a movable star) is rendered to a texture and then distorted in real time based on the mouse cursor position, representing the black hole.

---

### ⭐⭐⭐⭐⭐ (5 / 5)

- **[Ping from Scratch](./project/ping_from_scratch/)** — Just reimplementing the ping command from scratch.
- **[AES-128](./project/aes/)** — A **minimal implementation** of core AES-128 encryption & decryption steps.
- **[Liquid simulation](./project/liquid%20simulation/)** — A program that simulates water flowing through a grid of square cells using a very simple cellular model.
- **[Black Hole Ray Simulator - 3D](./project/black_hole_ray_interaction_3D/)** — The project is a 3D black hole simulator visualizing light ray trajectories, gravitational distortions, and an accretion disk using OpenGL and GLFW. It enables real-time interaction with camera controls, predefined scenarios, and physical parameters like the Schwarzschild radius.
- **[Black Hole Ray Tracer](./project/black_hole_light_interaction_3D/)** — The project simulates gravitational interactions between two stars and a central supermassive black hole, using ray tracing to render light bending according to the Schwarzschild metric of general relativity. It features interactive camera controls (orbit, pan, zoom), a deformable spacetime grid, an accretion disk, and options to pause physics or toggle the grid visibility.