// this is the actual implementation of all the matrix functions
// the declarations are in matrix.h and this file has the real code

// stdio is needed for printf in matrix_print and for file reading/writing
// stdlib is needed for malloc, calloc, and free
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

// creates a new matrix and allocates memory for it dynamically
// dynamic means we decide the size at runtime not at compile time
// this took me a while to understand but basically we need malloc/calloc
// because we dont know how big the matrix will be ahead of time
Matrix *matrix_create(int rows, int cols) {

    // make sure someone didnt pass in a weird size like 0 or negative
    // that would cause all kinds of problems later
    if (rows <= 0 || cols <= 0) return NULL;

    // allocate memory for the Matrix struct itself
    // sizeof(Matrix) gives us how many bytes the struct needs
    Matrix *m = malloc(sizeof(Matrix));

    // always check if malloc worked - on most computers it will
    // but its good practice and apparently important for real programs
    if (!m) return NULL;

    // store the dimensions inside the struct so we can check them later
    m->rows = rows;
    m->cols = cols;

    // now allocate an array of row pointers
    // each element here will point to one full row of doubles
    // i use calloc instead of malloc because calloc zeros everything out automatically
    m->data = calloc(rows, sizeof(double *));
    if (!m->data) {
        // if this fails we need to free the struct we already allocated
        // otherwise that memory just leaks and is gone forever
        free(m);
        return NULL;
    }

    // now allocate each actual row one by one
    for (int i = 0; i < rows; i++) {
        // calloc(cols, sizeof(double)) gives us one row of `cols` doubles all set to 0
        m->data[i] = calloc(cols, sizeof(double));

        if (!m->data[i]) {
            // if this fails mid-way through we have to clean up everything
            // we already allocated or we leak memory for all the rows we did so far
            // this was tricky to get right - we loop back through and free
            // only the rows that were successfully allocated (0 to i-1)
            for (int j = 0; j < i; j++) free(m->data[j]);
            free(m->data);
            free(m);
            return NULL;
        }
    }

    // if we made it here everything worked, return the finished matrix
    return m;
}

// frees all the memory that the matrix is using
// you have to free in the right order - first the individual rows,
// then the array of row pointers, then the struct itself
// if you do it in the wrong order you lose track of things
void matrix_free(Matrix *m) {

    // if someone passes NULL just do nothing instead of crashing
    if (!m) return;

    // free each individual row first
    for (int i = 0; i < m->rows; i++)
        free(m->data[i]);

    // then free the array that held all the row pointers
    free(m->data);

    // finally free the struct itself
    free(m);
}

// sets a single value inside the matrix at the given row and column
// remember rows and columns are zero-indexed so the first one is [0][0]
void matrix_set(Matrix *m, int row, int col, double val) {

    // check that the matrix exists and that the position is actually inside it
    // without this check we could write to random memory which is really bad
    if (!m || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;

    // actually set the value - this is the easy part
    m->data[row][col] = val;
}

// returns the value at a given position in the matrix
// marked const because we promise not to change anything
double matrix_get(const Matrix *m, int row, int col) {

    // same safety check as matrix_set - make sure position is valid
    // returning 0.0 instead of crashing seems like a reasonable fallback
    if (!m || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return 0.0;

    return m->data[row][col];
}

// prints the matrix to the terminal so you can see whats inside it
// the format looks like:  [   1.000   2.000   3.000 ]
// i picked 8.3f format which means 8 characters wide with 3 decimal places
// this makes the columns line up nicely even with big numbers
void matrix_print(const Matrix *m) {

    // if the matrix is NULL just say so and return
    if (!m) { printf("(null matrix)\n"); return; }

    // loop through every row
    for (int i = 0; i < m->rows; i++) {
        printf("[ ");

        // loop through every column in this row
        for (int j = 0; j < m->cols; j++)
            printf("%8.3f ", m->data[i][j]);  // print each number nicely formatted

        printf("]\n");  // close the bracket and go to next line
    }
}

// adds two matrices together element by element
// so result[i][j] = a[i][j] + b[i][j] for every position
// both matrices need to be the same size - you cant add a 2x3 to a 3x2
Matrix *matrix_add(const Matrix *a, const Matrix *b) {

    // check that both matrices exist and have the same dimensions
    // returning NULL is how we signal that something went wrong
    if (!a || !b || a->rows != b->rows || a->cols != b->cols) return NULL;

    // create a new matrix to store the result - same size as the inputs
    Matrix *result = matrix_create(a->rows, a->cols);
    if (!result) return NULL;

    // go through every single cell and add the two values together
    for (int i = 0; i < a->rows; i++)
        for (int j = 0; j < a->cols; j++)
            result->data[i][j] = a->data[i][j] + b->data[i][j];

    return result;
}

// subtracts matrix b from matrix a element by element
// so result[i][j] = a[i][j] - b[i][j]
// same size requirement as addition
Matrix *matrix_sub(const Matrix *a, const Matrix *b) {

    // same checks as add - both need to exist and be the same size
    if (!a || !b || a->rows != b->rows || a->cols != b->cols) return NULL;

    Matrix *result = matrix_create(a->rows, a->cols);
    if (!result) return NULL;

    // same as addition but subtract instead
    for (int i = 0; i < a->rows; i++)
        for (int j = 0; j < a->cols; j++)
            result->data[i][j] = a->data[i][j] - b->data[i][j];

    return result;
}

// multiplies two matrices together using the standard matrix multiplication algorithm
// this is NOT the same as multiplying element by element - its more complicated
// to get result[i][j] you take the dot product of row i from a and column j from b
// that means you multiply each pair and add them all up
//
// the size rule is: a must have the same number of columns as b has rows
// so if a is 2x3 and b is 3x5 then result will be 2x5
// if a is 2x3 and b is 2x3 it wont work and we return NULL
Matrix *matrix_mul(const Matrix *a, const Matrix *b) {

    // a->cols must equal b->rows or the math literally doesnt work
    if (!a || !b || a->cols != b->rows) return NULL;

    // result has rows from a and cols from b
    Matrix *result = matrix_create(a->rows, b->cols);
    if (!result) return NULL;

    // three nested loops - i is the row of the result,
    // j is the column of the result,
    // k is the index we use to compute the dot product
    // i used i-k-j order instead of i-j-k because apparently its faster
    // due to how the cpu cache works - i dont fully understand it yet
    for (int i = 0; i < a->rows; i++)
        for (int k = 0; k < a->cols; k++)
            for (int j = 0; j < b->cols; j++)
                // accumulate the dot product into result[i][j]
                // += means we keep adding to whatever was already there
                result->data[i][j] += a->data[i][k] * b->data[k][j];

    return result;
}

// reads a matrix from a plain text file
// the expected format is:
//   first line: two integers for rows and cols
//   remaining lines: space-separated decimal values, one row per line
// this lets us share data between the C program and python scripts
Matrix *matrix_from_file(const char *path) {

    // try to open the file for reading
    FILE *f = fopen(path, "r");
    if (!f) {
        // print an error so the user knows which file failed
        fprintf(stderr, "error: could not open file '%s'\n", path);
        return NULL;
    }

    // read the dimensions from the first line
    int rows, cols;
    if (fscanf(f, "%d %d", &rows, &cols) != 2) {
        fprintf(stderr, "error: could not read dimensions from '%s'\n", path);
        fclose(f);
        return NULL;
    }

    // create the matrix now that we know the size
    Matrix *m = matrix_create(rows, cols);
    if (!m) {
        fprintf(stderr, "error: could not allocate matrix from '%s'\n", path);
        fclose(f);
        return NULL;
    }

    // read every value from the file into the matrix
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double val;
            if (fscanf(f, "%lf", &val) != 1) {
                // the file didnt have enough numbers - something is wrong with it
                fprintf(stderr, "error: not enough values in '%s' (expected %d x %d)\n",
                        path, rows, cols);
                matrix_free(m);
                fclose(f);
                return NULL;
            }
            m->data[i][j] = val;
        }
    }

    fclose(f);
    return m;
}

// saves a matrix to a text file
// uses the same format that matrix_from_file expects
// so you can save a result and load it again later, or read it in python
int matrix_to_file(const Matrix *m, const char *path) {

    // sanity check - dont crash if someone passes NULL
    if (!m || !path) return -1;

    // open the file for writing - this creates the file if it doesnt exist
    // and overwrites it if it does
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "error: could not open '%s' for writing\n", path);
        return -1;
    }

    // write dimensions on the first line
    fprintf(f, "%d %d\n", m->rows, m->cols);

    // write each row on its own line with high precision
    // using 10 decimal places so we dont lose accuracy
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            fprintf(f, "%.10f", m->data[i][j]);
            // put a space between numbers but not after the last one
            if (j < m->cols - 1) fprintf(f, " ");
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return 0;  // 0 means success
}
