// this is the header file for my matrix library
// a header file basically just tells other files what functions exist
// without actually having the code for them - the real code is in matrix.c
// you include this file whenever you want to use the matrix stuff

// this part makes sure the file is only included once even if
// something accidentally includes it twice - i learned this is called
// an "include guard" and its apparently very important
#ifndef MATRIX_H
#define MATRIX_H

// this is the main structure that holds a matrix
// i used a double pointer (double **data) because the matrix size
// can be anything - its not fixed at compile time like a normal array would be
// rows and cols just store how big the matrix is so we dont lose track
typedef struct {
    int rows;      // how many rows the matrix has
    int cols;      // how many columns the matrix has
    double **data; // the actual numbers - its a pointer to an array of pointers
                   // basically each data[i] is one row, and data[i][j] is one cell
} Matrix;

// creates a new matrix with the given number of rows and columns
// all values start at zero because i use calloc inside
// returns NULL if something goes wrong (like not enough memory)
Matrix *matrix_create(int rows, int cols);

// frees all the memory that was allocated for the matrix
// super important to call this when youre done or you get memory leaks
// i forgot to do this at first and valgrind yelled at me
void matrix_free(Matrix *m);

// sets the value at position (row, col) to val
// row and col start at 0 not 1 - i kept making off by one errors with this
void matrix_set(Matrix *m, int row, int col, double val);

// gets the value at position (row, col) and returns it
// returns 0.0 if you give it bad indices so it doesnt crash
double matrix_get(const Matrix *m, int row, int col);

// prints the matrix to the terminal in a readable format
// useful for debugging to see if the values are what you expect
void matrix_print(const Matrix *m);

// adds two matrices together and returns the result as a new matrix
// both matrices need to be the exact same size for this to work
// returns NULL if the sizes dont match
Matrix *matrix_add(const Matrix *a, const Matrix *b);

// subtracts matrix b from matrix a and returns the result as a new matrix
// again both matrices need to be the same size
// returns NULL if sizes dont match
Matrix *matrix_sub(const Matrix *a, const Matrix *b);

// multiplies two matrices together - this one is more complicated
// the number of columns in a has to equal the number of rows in b
// for example a 2x3 matrix can multiply with a 3x4 matrix and you get a 2x4
// returns NULL if the dimensions are incompatible
Matrix *matrix_mul(const Matrix *a, const Matrix *b);

// reads a matrix from a text file
// the file format is: first line has rows and cols, then the values row by row
// example file:
//   3 3
//   1.0 2.0 3.0
//   4.0 5.0 6.0
//   7.0 8.0 9.0
// returns NULL if the file cant be opened or the format is wrong
Matrix *matrix_from_file(const char *path);

// saves a matrix to a text file using the same format as matrix_from_file
// this is useful for passing data between the C program and python scripts
// returns 0 on success, -1 if something went wrong
int matrix_to_file(const Matrix *m, const char *path);

// end of the include guard - matches the #ifndef at the top
#endif
