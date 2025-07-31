# Neural Network

A minimal multi‑layer perceptron that learns the XOR function using **stochastic gradient descent**, **binary cross‑entropy**, and **sigmoid activations**.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build
./nn            # random seed from time()
./nn 12345      # deterministic seed
```

> Needs only the C standard library + `-lm` for `exp`, `log`, `sqrt`.

Typical output every 500 epochs:

```
epoch 1    loss=0.703812  acc=50.00%
epoch 500  loss=0.021334  acc=100.00%
epoch 1000 loss=0.008951  acc=100.00%
...
=== Final results ===
Input: 0 0  -> Target: 0  Pred: 0.0103
Input: 1 0  -> Target: 1  Pred: 0.9834
Input: 0 1  -> Target: 1  Pred: 0.9891
Input: 1 1  -> Target: 0  Pred: 0.0122
```

---

## 2. Shapes & Constants

```c
enum { NUM_INPUTS = 2, HIDDEN_NODES = 2, NUM_OUTPUTS = 1, NUM_TRAIN = 4 };
#define EPOCHS   2000
#define LR       0.1
#define EPS      1e-12   // avoid log(0)
#define PATIENCE 500     // early stop if no better loss
```

* **Network topology**: 2 inputs → 2 hidden neurons → 1 output.
* **Training set**: 4 rows → truth table of XOR.
* **Learning rate**: 0.1 (fixed). Try lowering if loss oscillates.
* **Patience**: stop after 500 checks without improvement (every 500 epochs).

---

## 3. Model Structure

```c
typedef struct {
    double w_ih[NUM_INPUTS][HIDDEN_NODES];   // input → hidden
    double w_ho[HIDDEN_NODES][NUM_OUTPUTS]; // hidden → output
    double b_h[HIDDEN_NODES];               // hidden biases
    double b_o[NUM_OUTPUTS];                // output biases
} MLP;
```

All weights/biases are simple `double` arrays. No dynamic allocation, no pointers to pointers.

---

## 4. Activations

```c
static double sigmoid(double x)            { return 1.0 / (1.0 + exp(-x)); }
static double d_sigmoid_from_y(double y)   { return y * (1.0 - y); }
```

* `sigmoid(x)` returns $\sigma(x) = \frac{1}{1 + e^{-x}}$.
* Derivative uses `y = σ(x)`: `σ'(x) = y(1 - y)`. Handy to avoid recomputing exp.

---

## 5. Weight Init (Xavier/Glorot)

```c
static double xavier(int fan_in, int fan_out) {
    double limit = sqrt(6.0 / (fan_in + fan_out));
    return (urand() * 2.0 - 1.0) * limit;
}
```

* Draws from `[-limit, +limit]` where `limit = √(6/(fan_in+fan_out))`.
* `urand()` is just `rand()/RAND_MAX`.
* This keeps initial activations in a reasonable range so gradients don’t vanish/explode immediately.

`init_net()` fills every weight with Xavier values; biases start at 0.

---

## 6. Forward Pass

```c
static void forward(const MLP *n, const double *x, double *h, double *o) {
    // Hidden layer
    for (int j = 0; j < HIDDEN_NODES; ++j) {
        double a = n->b_h[j];
        for (int i = 0; i < NUM_INPUTS; ++i)
            a += x[i] * n->w_ih[i][j];
        h[j] = sigmoid(a);
    }
    // Output layer
    for (int j = 0; j < NUM_OUTPUTS; ++j) {
        double a = n->b_o[j];
        for (int i = 0; i < HIDDEN_NODES; ++i)
            a += h[i] * n->w_ho[i][j];
        o[j] = sigmoid(a);
    }
}
```

* Compute affine transform + sigmoid for each layer.
* `h` and `o` buffers are passed in (stack arrays in `main`).

---

## 7. Loss: Binary Cross‑Entropy

```c
static double bce_loss(const double *t, const double *o) {
    double y = o[0], target = t[0];
    return -(target * log(y + EPS) + (1.0 - target) * log(1.0 - y + EPS));
}
```

For a single output:

$$
L = -\big(t\log y + (1-t)\log(1-y)\big)
$$

`EPS` avoids `log(0)`.

---

## 8. Backpropagation (SGD Update)

```c
static void backward(MLP *n, const double *x, const double *t,
                     const double *h, const double *o) {
    double delta_o[NUM_OUTPUTS];
    for (int j = 0; j < NUM_OUTPUTS; ++j)
        delta_o[j] = o[j] - t[j];   // BCE + sigmoid grad simplifies

    double delta_h[HIDDEN_NODES] = {0};
    for (int i = 0; i < HIDDEN_NODES; ++i) {
        double err = 0.0;
        for (int j = 0; j < NUM_OUTPUTS; ++j)
            err += delta_o[j] * n->w_ho[i][j];
        delta_h[i] = err * d_sigmoid_from_y(h[i]);
    }

    // Update output layer
    for (int j = 0; j < NUM_OUTPUTS; ++j) {
        n->b_o[j] -= LR * delta_o[j];
        for (int i = 0; i < HIDDEN_NODES; ++i)
            n->w_ho[i][j] -= LR * h[i] * delta_o[j];
    }

    // Update hidden layer
    for (int j = 0; j < HIDDEN_NODES; ++j) {
        n->b_h[j] -= LR * delta_h[j];
        for (int i = 0; i < NUM_INPUTS; ++i)
            n->w_ih[i][j] -= LR * x[i] * delta_h[j];
    }
}
```

Key ideas:

* With sigmoid + BCE, output delta simplifies to `o − t`.
* Hidden deltas = weighted sum of downstream deltas × derivative of hidden activations.
* Stochastic: updates per sample, not per batch.

---

## 9. Training Loop & Early Stopping

```c
for (int epoch = 1; epoch <= EPOCHS; ++epoch) {
    shuffle(order, NUM_TRAIN);
    for (int idx = 0; idx < NUM_TRAIN; ++idx) {
        int k = order[idx];
        forward(&net, X[k], h, o);
        backward(&net, X[k], Y[k], h, o);
    }
    if (epoch % 500 == 0 || epoch == 1) {
        // compute avg loss & accuracy
        ...
        if (loss + 1e-6 < best_loss) best_loss = loss, epochs_no_improve = 0;
        else if (++epochs_no_improve > PATIENCE) { printf("Early stopping...\n"); break; }
    }
}
```

* `shuffle()` permutes the 4 training samples (Fisher–Yates) each epoch.
* Evaluate periodically (every 500 epochs) to avoid extra compute.
* Track best loss; stop if it stagnates for `PATIENCE` checks.

---

## 10. Final Report

```c
puts("\n=== Final results ===");
for (int n = 0; n < NUM_TRAIN; ++n) {
    forward(&net, X[n], h, o);
    printf("Input: %.0f %.0f  -> Target: %.0f  Pred: %.4f\n",
           X[n][0], X[n][1], Y[n][0], o[0]);
}
```

* Prints raw sigmoid output (`0–1`). You can threshold at 0.5 for class labels.