// this program benchmarks how fast our matrix multiplication is
// it tests several different matrix sizes and measures how long each one takes
// the results can be compared against pytorch to see how we stack up
//
// to compile and run:
//   gcc -O2 -o benchmark benchmark.c matrix.c -lm
//   ./benchmark
//
// note: we use -O2 to turn on compiler optimizations
// without -O2 the results would be much slower and not a fair comparison

#include <stdio.h>
#include <stdlib.h>
#include <time.h>    // for clock_gettime - high precision timing
#include <math.h>    // for fabs - absolute value for doubles
#include "matrix.h"

// fills a matrix with random values between -10.0 and 10.0
// we use a fixed random seed so the results are reproducible
static void matrix_fill_random(Matrix *m) {
    for (int i = 0; i < m->rows; i++)
        for (int j = 0; j < m->cols; j++)
            // rand() gives us 0 to RAND_MAX, we scale it to -10 to 10
            m->data[i][j] = ((double)rand() / RAND_MAX) * 20.0 - 10.0;
}

// returns the current time in seconds with nanosecond precision
// clock_gettime is more accurate than just using clock()
// CLOCK_MONOTONIC means it wont jump around if the system clock changes
static double get_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    // convert nanoseconds to fractional seconds and add to whole seconds
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

// runs a matrix multiplication benchmark for a specific size
// does multiple iterations for small matrices to get more stable timing
// returns the average time in milliseconds
static double run_mul_benchmark(int n, int iterations) {
    // create two n x n matrices with random values
    Matrix *A = matrix_create(n, n);
    Matrix *B = matrix_create(n, n);
    matrix_fill_random(A);
    matrix_fill_random(B);

    // do a warmup run first - sometimes the first run is slower
    // because of cache warmup effects and other OS stuff
    Matrix *warmup = matrix_mul(A, B);
    matrix_free(warmup);

    // now do the actual timed runs
    double t_start = get_time_seconds();
    for (int it = 0; it < iterations; it++) {
        Matrix *C = matrix_mul(A, B);
        matrix_free(C);  // free each result or we run out of memory
    }
    double t_end = get_time_seconds();

    matrix_free(A);
    matrix_free(B);

    // return average time per multiplication in milliseconds
    return ((t_end - t_start) / iterations) * 1000.0;
}

// for matrix multiplication of two N x N matrices,
// the number of floating point operations is approximately 2 * N^3
// (N^2 dot products, each requiring N multiplications and N additions)
// GFLOPS = billion floating point operations per second
static double compute_gflops(int n, double time_ms) {
    double flops = 2.0 * (double)n * (double)n * (double)n;
    double time_seconds = time_ms / 1000.0;
    return flops / (time_seconds * 1.0e9);
}

int main(void) {
    // fix the random seed so we always get the same matrices
    // this makes it easier to reproduce results
    srand(42);

    // the matrix sizes we want to test
    // larger sizes take much longer due to the cubic time complexity
    int sizes[]      = {32, 64, 128, 256, 512, 1024};
    int n_sizes      = 6;

    // how many times to repeat each multiplication for a stable average
    // we run small matrices more times because they finish too quickly
    // to measure accurately with just one run
    int iterations[] = {50, 20, 10, 5, 3, 1};

    printf("=============================================================\n");
    printf("  C Matrix Multiplication Benchmark (naive triple-loop)\n");
    printf("  Compiled with -O2 optimization\n");
    printf("=============================================================\n");
    printf("%-8s  %-14s  %-12s  %-10s\n",
           "Size", "Avg Time (ms)", "GFLOPS", "Iterations");
    printf("%-8s  %-14s  %-12s  %-10s\n",
           "--------", "--------------", "------------", "----------");

    for (int s = 0; s < n_sizes; s++) {
        int n    = sizes[s];
        int iters = iterations[s];

        // run the benchmark for this size
        double avg_ms = run_mul_benchmark(n, iters);
        double gflops = compute_gflops(n, avg_ms);

        printf("%-8d  %-14.3f  %-12.4f  %-10d\n", n, avg_ms, gflops, iters);

        // flush output immediately so we can see progress while larger sizes run
        fflush(stdout);
    }

    printf("=============================================================\n");
    printf("Note: PyTorch uses optimized BLAS routines (MKL/OpenBLAS)\n");
    printf("and potentially GPU acceleration, so expect it to be much\n");
    printf("faster than this naive implementation, especially for large N.\n");
    printf("Run validate_and_benchmark.py for the comparison.\n");

    return 0;
}
