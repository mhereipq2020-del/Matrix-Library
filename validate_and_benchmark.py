# this script does two things:
#   1. VALIDATION: checks that our C matrix library gives the same
#      answers as pytorch for add, subtract, and multiply
#   2. BENCHMARKING: compares how fast our C code is vs pytorch
#
# how to run:
#   python3 validate_and_benchmark.py
#
# requirements:
#   - compile the C program first: make  (or: gcc -O2 -o matrix_demo matrix.c main.c)
#   - pip install torch numpy

import os
import sys
import time
import subprocess
import numpy as np
import torch

# -----------------------------------------------------------------------
# FILE I/O HELPERS
# these read/write matrices in the same format our C library uses
# format: first line is "rows cols", then values row by row
# -----------------------------------------------------------------------

def write_matrix_file(matrix, path):
    """Save a numpy matrix to a text file that our C program can read."""
    rows, cols = matrix.shape
    with open(path, 'w') as f:
        f.write(f"{rows} {cols}\n")
        for i in range(rows):
            # write each row as space-separated values with lots of decimal places
            # so we dont lose precision when C reads them back
            row_str = " ".join(f"{v:.10f}" for v in matrix[i])
            f.write(row_str + "\n")


def read_matrix_file(path):
    """Load a matrix from a text file written by our C program."""
    with open(path, 'r') as f:
        rows, cols = map(int, f.readline().split())
        data = []
        for _ in range(rows):
            row = list(map(float, f.readline().split()))
            data.append(row)
    return np.array(data, dtype=np.float64)


# -----------------------------------------------------------------------
# C PROGRAM RUNNER
# calls our compiled C binary with file arguments
# -----------------------------------------------------------------------

def run_c_operation(op, file_a, file_b, file_out):
    """
    Run the C matrix_demo program in file mode.
    Returns True if it succeeded, False if there was an error.
    """
    # make sure the binary exists before trying to run it
    if not os.path.exists("./matrix_demo"):
        print("ERROR: ./matrix_demo not found. Run 'make' first.")
        return False

    result = subprocess.run(
        ["./matrix_demo", op, file_a, file_b, file_out],
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        print(f"  C program error: {result.stderr.strip()}")
        return False

    return True


# -----------------------------------------------------------------------
# VALIDATION
# generates random test cases, runs them through our C code and pytorch,
# then checks that the answers match within a small tolerance
# -----------------------------------------------------------------------

def validate_all_operations():
    """
    Tests add, sub, and mul for several matrix sizes.
    Prints PASS or FAIL for each test along with the maximum difference.
    """
    print("=" * 60)
    print("  VALIDATION: C Library vs PyTorch")
    print("=" * 60)

    # we use a fixed seed so tests are reproducible
    rng = np.random.default_rng(seed=42)

    # test cases: (rows_a, cols_a, rows_b, cols_b)
    # for add and sub we need matching sizes, for mul we need a_cols == b_rows
    test_cases = [
        # small square matrices - easy to debug if something goes wrong
        (2, 2, 2, 2),
        (3, 3, 3, 3),
        (4, 4, 4, 4),
        # medium sizes
        (10, 10, 10, 10),
        (50, 50, 50, 50),
        # rectangular matrices - tests that the dimension rules work right
        (3, 5, 5, 4),   # mul only: 3x5 * 5x4 = 3x4
        (8, 12, 12, 6), # mul only: 8x12 * 12x6 = 8x6
        # larger sizes to make sure file I/O works with big data
        (100, 100, 100, 100),
        (200, 200, 200, 200),
    ]

    # make sure temp files go somewhere sensible
    os.makedirs("data", exist_ok=True)
    file_a   = "data/tmp_a.txt"
    file_b   = "data/tmp_b.txt"
    file_out = "data/tmp_out.txt"

    all_passed = True

    for (ra, ca, rb, cb) in test_cases:
        # generate random matrices with values in [-5, 5]
        A_np = rng.uniform(-5, 5, size=(ra, ca))
        B_np = rng.uniform(-5, 5, size=(rb, cb))

        A_torch = torch.tensor(A_np, dtype=torch.float64)
        B_torch = torch.tensor(B_np, dtype=torch.float64)

        # write both matrices to files so C can read them
        write_matrix_file(A_np, file_a)
        write_matrix_file(B_np, file_b)

        # decide which operations to test for this size combination
        # add and sub only work when shapes are identical
        ops_to_test = ["mul"]  # mul works if ra x ca and ca == rb
        if ra == rb and ca == cb:
            ops_to_test = ["add", "sub", "mul"]

        for op in ops_to_test:
            # skip mul if dimensions are incompatible (ca must equal rb)
            if op == "mul" and ca != rb:
                continue

            # compute the expected answer using pytorch
            if op == "add":
                expected = (A_torch + B_torch).numpy()
            elif op == "sub":
                expected = (A_torch - B_torch).numpy()
            elif op == "mul":
                expected = (A_torch @ B_torch).numpy()

            # run our C program on the same input files
            ok = run_c_operation(op, file_a, file_b, file_out)
            if not ok:
                print(f"  FAIL  {op:3s}  ({ra}x{ca}) op ({rb}x{cb})  -- C program failed")
                all_passed = False
                continue

            # load what C wrote back
            c_result = read_matrix_file(file_out)

            # compare - floating point math is never exactly equal
            # so we check that the difference is very small
            max_diff = np.max(np.abs(expected - c_result))
            tolerance = 1e-6  # differences smaller than this are fine

            status = "PASS" if max_diff < tolerance else "FAIL"
            if status == "FAIL":
                all_passed = False

            print(f"  {status}  {op:3s}  ({ra}x{ca}) op ({rb}x{cb})  "
                  f"max_diff={max_diff:.2e}  {'OK' if max_diff < tolerance else 'TOO LARGE'}")

    print()
    if all_passed:
        print("  All tests PASSED - C results match PyTorch within tolerance 1e-6")
    else:
        print("  Some tests FAILED - check the output above")
    print()

    # clean up temp files
    for f in [file_a, file_b, file_out]:
        if os.path.exists(f):
            os.remove(f)


# -----------------------------------------------------------------------
# BENCHMARKING
# measures wall clock time for matrix multiplication in C vs pytorch
# -----------------------------------------------------------------------

def benchmark_c(sizes, n_repeats):
    """
    Times our C matrix multiplication by calling it as a subprocess
    with randomly generated matrix files.
    Returns a dict of size -> time_ms.
    """
    rng = np.random.default_rng(seed=0)
    results = {}

    os.makedirs("data", exist_ok=True)
    file_a   = "data/bench_a.txt"
    file_b   = "data/bench_b.txt"
    file_out = "data/bench_out.txt"

    for n in sizes:
        # generate and save random matrices once (file I/O time not included in timing)
        A = rng.uniform(-1, 1, size=(n, n))
        B = rng.uniform(-1, 1, size=(n, n))
        write_matrix_file(A, file_a)
        write_matrix_file(B, file_b)

        # time how long the C program takes across several runs
        times = []
        for _ in range(n_repeats):
            t0 = time.perf_counter()
            subprocess.run(
                ["./matrix_demo", "mul", file_a, file_b, file_out],
                capture_output=True  # suppress output
            )
            t1 = time.perf_counter()
            times.append((t1 - t0) * 1000.0)  # convert to ms

        # use the minimum time - thats the most accurate for benchmarking
        # (maximum includes OS scheduling noise and other interruptions)
        results[n] = min(times)

    # clean up
    for f in [file_a, file_b, file_out]:
        if os.path.exists(f):
            os.remove(f)

    return results


def benchmark_pytorch(sizes, n_repeats, use_gpu=False):
    """
    Times pytorch matrix multiplication.
    Can optionally use GPU if available.
    Returns a dict of size -> time_ms.
    """
    device = "cpu"
    if use_gpu and torch.cuda.is_available():
        device = "cuda"

    rng = torch.Generator(device=device)
    rng.manual_seed(0)

    results = {}

    for n in sizes:
        A = torch.rand(n, n, dtype=torch.float64, device=device, generator=rng)
        B = torch.rand(n, n, dtype=torch.float64, device=device, generator=rng)

        # warmup run - pytorch sometimes compiles things on first use
        _ = torch.mm(A, B)
        if device == "cuda":
            torch.cuda.synchronize()  # make sure GPU is done before timing

        times = []
        for _ in range(n_repeats):
            t0 = time.perf_counter()
            C = torch.mm(A, B)
            if device == "cuda":
                torch.cuda.synchronize()  # wait for GPU to actually finish
            t1 = time.perf_counter()
            times.append((t1 - t0) * 1000.0)

        results[n] = min(times)

    return results


def gflops(n, time_ms):
    """Compute GFLOPS from matrix size and time."""
    flops = 2.0 * n**3  # approximately 2*N^3 operations for NxN multiplication
    return flops / (time_ms / 1000.0) / 1e9


def run_benchmark():
    """Runs the full benchmark and prints a comparison table."""
    print("=" * 60)
    print("  BENCHMARK: C Library vs PyTorch (CPU)")
    if torch.cuda.is_available():
        print(f"  GPU available: {torch.cuda.get_device_name(0)}")
    else:
        print("  No GPU detected - running CPU only")
    print("=" * 60)

    # decide which sizes to test and how many repeats
    # note: C is much slower than pytorch so we use fewer repeats for large sizes
    sizes_c      = [32, 64, 128, 256, 512]
    sizes_torch  = [32, 64, 128, 256, 512, 1024, 2048]

    # more repeats for small matrices (they finish too fast for one run)
    repeats_c     = {32: 10, 64: 5, 128: 3, 256: 2, 512: 1}
    repeats_torch = {32: 50, 64: 50, 128: 30, 256: 20, 512: 10, 1024: 5, 2048: 3}

    print("\nRunning C benchmarks (this may take a minute for larger sizes)...")
    c_times = {}
    for n in sizes_c:
        c_times[n] = benchmark_c([n], repeats_c[n])[n]
        print(f"  C   {n:5d}x{n}: {c_times[n]:8.2f} ms  ({gflops(n, c_times[n]):.4f} GFLOPS)")

    print("\nRunning PyTorch CPU benchmarks...")
    torch_cpu_times = {}
    for n in sizes_torch:
        torch_cpu_times[n] = benchmark_pytorch([n], repeats_torch[n], use_gpu=False)[n]
        print(f"  PT  {n:5d}x{n}: {torch_cpu_times[n]:8.4f} ms  ({gflops(n, torch_cpu_times[n]):.4f} GFLOPS)")

    torch_gpu_times = {}
    if torch.cuda.is_available():
        print("\nRunning PyTorch GPU benchmarks...")
        for n in sizes_torch:
            torch_gpu_times[n] = benchmark_pytorch([n], repeats_torch[n], use_gpu=True)[n]
            print(f"  GPU {n:5d}x{n}: {torch_gpu_times[n]:8.4f} ms  ({gflops(n, torch_gpu_times[n]):.4f} GFLOPS)")

    # print a combined comparison table for the sizes where we have both
    common = [n for n in sizes_c if n in torch_cpu_times]
    print()
    print("=" * 70)
    print("  COMPARISON TABLE")
    print("=" * 70)

    header = f"{'Size':>8}  {'C (ms)':>10}  {'PT CPU (ms)':>12}  {'Speedup (PT/C)':>15}"
    if torch_gpu_times:
        header += f"  {'PT GPU (ms)':>12}  {'GPU Speedup':>12}"
    print(header)
    print("-" * len(header))

    for n in common:
        speedup_cpu = c_times[n] / torch_cpu_times[n]
        line = (f"{n:>8}  {c_times[n]:>10.2f}  "
                f"{torch_cpu_times[n]:>12.4f}  {speedup_cpu:>14.1f}x")
        if torch_gpu_times and n in torch_gpu_times:
            speedup_gpu = c_times[n] / torch_gpu_times[n]
            line += f"  {torch_gpu_times[n]:>12.4f}  {speedup_gpu:>11.1f}x"
        print(line)

    print()
    print("WHY IS PYTORCH SO MUCH FASTER?")
    print("-" * 40)
    print("Our C code uses a simple triple nested loop - it does one")
    print("multiplication and one addition at a time.")
    print()
    print("PyTorch uses highly optimized BLAS libraries (like OpenBLAS or")
    print("Intel MKL) that use techniques like:")
    print("  - SIMD/vectorization: process multiple numbers at once using")
    print("    special CPU instructions (SSE, AVX)")
    print("  - Cache blocking: reorganize the computation so data stays")
    print("    in fast CPU cache instead of going to slow RAM")
    print("  - Multi-threading: use multiple CPU cores in parallel")
    if torch.cuda.is_available():
        print("  - GPU: run thousands of operations simultaneously")
    print()
    print("Our implementation is O(N^3) with a large constant factor.")
    print("PyTorch is also O(N^3) but the constant factor is much smaller.")


# -----------------------------------------------------------------------
# MAIN
# -----------------------------------------------------------------------

if __name__ == "__main__":
    # make sure we're running from the right directory
    if not os.path.exists("./matrix_demo"):
        print("ERROR: ./matrix_demo not found.")
        print("Please run 'make' (or 'gcc -O2 -o matrix_demo matrix.c main.c') first.")
        sys.exit(1)

    print()
    validate_all_operations()

    print()
    run_benchmark()
