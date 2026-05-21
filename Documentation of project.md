# Matrix Library

A lightweight, high-performance C library for dynamic matrix arithmetic. 

Performing matrix arithmetic by hand is slow and error-prone, especially for matrices larger than 2×2. Existing solutions often require a heavy runtime (like Python or MATLAB) or are too low-level for quick integration. This library bridges the gap by providing a fast, dependency-free C implementation with easy-to-use interactive and file-based I/O, validated against PyTorch.

---

## Goals

* **Correctness & Readability:** Provide correct, clearly readable implementations of matrix addition, subtraction, and multiplication.
* **Flexible I/O:** Allow matrices to be entered interactively in the terminal or loaded from plain text files.
* **Validation:** Validate correctness against an established machine learning library (PyTorch).
* **Benchmarking:** Provide honest, transparent performance benchmarking.

---

## Main Features

* **Dynamic Matrix Allocation:** Create and manipulate matrices of any size at runtime.
* **Core Arithmetic Operations:** * `add` (Addition)
  * `sub` (Subtraction)
  * `mul` (Multiplication)
* **Interactive Input Mode:** Enter matrix values directly by typing them into the terminal.
* **File I/O Mode:** Read matrices from text files and optionally save the results back to disk.
* **C Benchmark:** Measure raw multiplication speed across multiple matrix sizes.
* **PyTorch Validation:** Confirm C results match PyTorch outputs to within standard floating-point tolerance.
* **PyTorch Benchmark Comparison:** Side-by-side speed comparison with PyTorch, including GPU performance.

---

## Error Handling & Robustness

The library is designed to fail gracefully and prevent segmentation faults under the following conditions:

| Issue | Library Behavior |
| :--- | :--- |
| **Incompatible dimensions** | Prints a clear error message explaining the dimensional requirements for the operation. |
| **File cannot be opened** | Prints the specific filename that failed to load. |
| **Invalid file format / missing values** | Identifies and prints which file caused the formatting issue. |
| **`NULL` matrix passed to a function** | Results in a safe no-op or returns `NULL`; **never crashes**. |
| **Out-of-bounds `get`/`set` index** | Results in a safe no-op or returns `0.0`. |
| **Memory allocation failure** | Gracefully returns `NULL` up the call stack. |
