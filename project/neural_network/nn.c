#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Simple XOR neural network with 2 input nodes, 2 hidden neurons, and 1 output node

enum { NUM_INPUTS = 2, HIDDEN_NODES = 2, NUM_OUTPUTS = 1, NUM_TRAIN = 4 };
#define EPOCHS   2000
#define LR       0.1      // Learning rate
#define EPS      1e-12    // Epsilon to prevent log(0)
#define PATIENCE 500      // Early stopping if no improvement

// Multi-Layer Perceptron structure
typedef struct {
    double w_ih[NUM_INPUTS][HIDDEN_NODES]; // input to hidden weights
    double w_ho[HIDDEN_NODES][NUM_OUTPUTS]; // hidden to output weights
    double b_h[HIDDEN_NODES]; // biases for hidden layer
    double b_o[NUM_OUTPUTS];  // biases for output layer
} MLP;

// Activation functions and derivatives
static double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }
static double d_sigmoid_from_y(double y) { return y * (1.0 - y); }

// Random initialization with Xavier/Glorot method
static double urand(void) { return (double)rand() / RAND_MAX; }
static double xavier(int fan_in, int fan_out) {
    double limit = sqrt(6.0 / (fan_in + fan_out));
    return (urand() * 2.0 - 1.0) * limit;
}

// Initialize weights and biases
static void init_net(MLP *n) {
    for (int i = 0; i < NUM_INPUTS; ++i)
        for (int j = 0; j < HIDDEN_NODES; ++j)
            n->w_ih[i][j] = xavier(NUM_INPUTS, HIDDEN_NODES);
    for (int i = 0; i < HIDDEN_NODES; ++i)
        for (int j = 0; j < NUM_OUTPUTS; ++j)
            n->w_ho[i][j] = xavier(HIDDEN_NODES, NUM_OUTPUTS);
    for (int j = 0; j < HIDDEN_NODES; ++j) n->b_h[j] = 0.0;
    for (int j = 0; j < NUM_OUTPUTS;  ++j) n->b_o[j] = 0.0;
}

// Forward pass: compute hidden and output activations
static void forward(const MLP *n, const double *x, double *h, double *o) {
    for (int j = 0; j < HIDDEN_NODES; ++j) {
        double a = n->b_h[j];
        for (int i = 0; i < NUM_INPUTS; ++i)
            a += x[i] * n->w_ih[i][j];
        h[j] = sigmoid(a);
    }

    for (int j = 0; j < NUM_OUTPUTS; ++j) {
        double a = n->b_o[j];
        for (int i = 0; i < HIDDEN_NODES; ++i)
            a += h[i] * n->w_ho[i][j];
        o[j] = sigmoid(a);
    }
}

// Backward pass: compute gradients and update weights (SGD)
static void backward(MLP *n, const double *x, const double *t,
                     const double *h, const double *o) {
    double delta_o[NUM_OUTPUTS];

    // Derivative of binary cross-entropy with sigmoid: delta = output - target
    for (int j = 0; j < NUM_OUTPUTS; ++j)
        delta_o[j] = o[j] - t[j];

    double delta_h[HIDDEN_NODES] = {0};
    for (int i = 0; i < HIDDEN_NODES; ++i) {
        double err = 0.0;
        for (int j = 0; j < NUM_OUTPUTS; ++j)
            err += delta_o[j] * n->w_ho[i][j];
        delta_h[i] = err * d_sigmoid_from_y(h[i]);
    }

    // Update output weights and biases
    for (int j = 0; j < NUM_OUTPUTS; ++j) {
        n->b_o[j] -= LR * delta_o[j];
        for (int i = 0; i < HIDDEN_NODES; ++i)
            n->w_ho[i][j] -= LR * h[i] * delta_o[j];
    }

    // Update hidden weights and biases
    for (int j = 0; j < HIDDEN_NODES; ++j) {
        n->b_h[j] -= LR * delta_h[j];
        for (int i = 0; i < NUM_INPUTS; ++i)
            n->w_ih[i][j] -= LR * x[i] * delta_h[j];
    }
}

// Binary cross-entropy loss
static double bce_loss(const double *t, const double *o) {
    double y = o[0], target = t[0];
    return -(target * log(y + EPS) + (1.0 - target) * log(1.0 - y + EPS));
}

// Fisher-Yates shuffle
static void shuffle(int *a, int n) {
    for (int i = 0; i < n - 1; ++i) {
        int j = i + rand() % (n - i);
        int tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
}

int main(int argc, char **argv) {
    unsigned seed = (argc > 1) ? (unsigned)strtoul(argv[1], NULL, 10)
                               : (unsigned)time(NULL);
    srand(seed);

    // XOR truth table
    double X[NUM_TRAIN][NUM_INPUTS]  = {{0,0},{1,0},{0,1},{1,1}};
    double Y[NUM_TRAIN][NUM_OUTPUTS] = {{0},{1},{1},{0}};

    MLP net;
    init_net(&net);

    int order[NUM_TRAIN] = {0,1,2,3};
    double best_loss = 1e9;
    int epochs_no_improve = 0;

    double h[HIDDEN_NODES], o[NUM_OUTPUTS];

    for (int epoch = 1; epoch <= EPOCHS; ++epoch) {
        shuffle(order, NUM_TRAIN);

        // SGD training over each sample
        for (int idx = 0; idx < NUM_TRAIN; ++idx) {
            int k = order[idx];
            forward(&net, X[k], h, o);
            backward(&net, X[k], Y[k], h, o);
        }

        // Evaluation every 500 epochs (or first epoch)
        if (epoch % 500 == 0 || epoch == 1) {
            double loss = 0.0;
            int correct = 0;
            for (int n = 0; n < NUM_TRAIN; ++n) {
                forward(&net, X[n], h, o);
                loss += bce_loss(Y[n], o);
                correct += ((o[0] > 0.5) == (Y[n][0] > 0.5));
            }
            loss /= NUM_TRAIN;
            printf("epoch %d  loss=%.6f  acc=%.2f%%\n", epoch, loss, 100.0 * correct / NUM_TRAIN);

            if (loss + 1e-6 < best_loss) {
                best_loss = loss;
                epochs_no_improve = 0;
            } else if (++epochs_no_improve > PATIENCE) {
                printf("Early stopping at epoch %d\n", epoch);
                break;
            }
        }
    }

    puts("\n=== Final results ===");
    for (int n = 0; n < NUM_TRAIN; ++n) {
        forward(&net, X[n], h, o);
        printf("Input: %.0f %.0f  -> Target: %.0f  Pred: %.4f\n",
               X[n][0], X[n][1], Y[n][0], o[0]);
    }

    return 0;
}