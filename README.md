# Matrix Library

A dynamic 2D matrix library written in C. Supports addition,
subtraction, and multiplication for matrices of any size. Includes file
I/O, a C benchmark, and a Python validation script that checks results
against PyTorch.

------------------------------------------------------------------------

## Requirements

- GCC
- Python 3 + PyTorch + NumPy (only needed for validation/benchmarking)

``` bash
pip install torch numpy
```

------------------------------------------------------------------------

## Build

``` bash
make
```

This compiles two binaries: `matrix_demo` (the main program) and
`benchmark` (the C speed test).

------------------------------------------------------------------------

## Run

### Interactive --- type your own matrices

``` bash
./matrix_demo
```

You will be prompted to choose an operation (`add`, `sub`, `mul`), then
enter the dimensions and values for both matrices.

### File mode --- load matrices from files

``` bash
./matrix_demo <op> <fileA> <fileB>
./matrix_demo <op> <fileA> <fileB> <outfile>   # also saves result to a file
```

Example using the included sample files:

``` bash
./matrix_demo mul data/matrix_a.txt data/matrix_b.txt
```

**File format** --- plain text, first line is dimensions, then values
row by row:

    3 3
    1.0 2.0 3.0
    4.0 5.0 6.0
    7.0 8.0 9.0

### C benchmark

``` bash
./benchmark
# or: make bench
```

Tests multiplication at sizes 32×32 through 1024×1024 and reports time
and GFLOPS.

### Validation + PyTorch benchmark

``` bash
python3 validate_and_benchmark.py
# or: make validate
```

Runs 23 automated test cases (add, sub, mul at various sizes and shapes)
comparing C output against PyTorch, then prints a speed comparison table
for C vs PyTorch CPU vs PyTorch GPU.

------------------------------------------------------------------------

## Project Structure

    matrix_lib/
    ├── matrix.h                   public API
    ├── matrix.c                   library implementation
    ├── main.c                     interactive and file-mode program
    ├── benchmark.c                C performance benchmark
    ├── validate_and_benchmark.py  PyTorch validation and benchmark
    ├── Makefile
    ├── DOCUMENTATION.md           full PRD / FSPEC / DEVSPEC / testing / benchmarking
    └── data/
        ├── matrix_a.txt           sample 3×3 matrix
        └── matrix_b.txt           sample 3×3 matrix

------------------------------------------------------------------------

## Clean up

``` bash
make clean
```
