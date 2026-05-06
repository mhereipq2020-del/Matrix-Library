# Makefile for the matrix library
# run 'make' to build everything
# run 'make benchmark' to build and run the C benchmark
# run 'make validate' to run the python validation + benchmark

CC     = gcc
CFLAGS = -Wall -Wextra -O2

# default target builds both binaries
all: matrix_demo benchmark

# the main interactive/file-mode program
matrix_demo: matrix.c main.c matrix.h
	$(CC) $(CFLAGS) -o matrix_demo matrix.c main.c

# the standalone C benchmark program
benchmark: matrix.c benchmark.c matrix.h
	$(CC) $(CFLAGS) -o benchmark matrix.c benchmark.c -lm

# run the C benchmark
bench: benchmark
	./benchmark

# run the python validation and comparison benchmark
validate: matrix_demo
	python3 validate_and_benchmark.py

# remove compiled binaries
clean:
	rm -f matrix_demo benchmark

.PHONY: all bench validate clean
