// this is the main program - it supports two modes:
//
//   INTERACTIVE MODE (no arguments):
//     ./matrix_demo
//     -- asks you to type in your operation and matrix values
//
//   FILE MODE (command line arguments):
//     ./matrix_demo <op> <fileA> <fileB>
//     -- reads matrices from files and prints the result
//
//     ./matrix_demo <op> <fileA> <fileB> <outfile>
//     -- reads matrices from files and saves the result to another file
//
//   where <op> is one of: add, sub, mul

// stdio is for printf and scanf
// string is for strcmp which lets us compare strings
// matrix.h has all the matrix stuff we wrote
#include <stdio.h>
#include <string.h>
#include "matrix.h"

// prints instructions for how to use this program
// called when the user passes the wrong number of arguments
static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s                               interactive mode\n", prog);
    printf("  %s <op> <fileA> <fileB>          file mode, print result\n", prog);
    printf("  %s <op> <fileA> <fileB> <out>    file mode, save result to file\n", prog);
    printf("\n");
    printf("  op can be: add, sub, mul\n");
    printf("\nFile format (plain text):\n");
    printf("  first line: rows cols\n");
    printf("  then values row by row, space separated\n");
    printf("  example:\n");
    printf("    3 3\n");
    printf("    1.0 2.0 3.0\n");
    printf("    4.0 5.0 6.0\n");
    printf("    7.0 8.0 9.0\n");
}

// this function handles reading one matrix from the user by typing values in
// i made it a separate function because we need to do it twice (for A and B)
// and i didnt want to copy paste the same code twice - my teacher said thats bad
// the "name" parameter is just so we can print "matrix A" or "matrix B"
// static means this function is only visible in this file which is fine
static Matrix *read_matrix_interactive(const char *name) {
    int rows, cols;

    // ask the user how big the matrix should be
    printf("Enter dimensions for matrix %s (rows cols): ", name);

    // scanf reads two integers from what the user types
    // the user should type something like "3 3" or "2 4"
    scanf("%d %d", &rows, &cols);

    // actually create the matrix in memory using our library function
    Matrix *m = matrix_create(rows, cols);

    // matrix_create returns NULL if something went wrong like bad dimensions
    if (!m) { printf("Failed to allocate matrix.\n"); return NULL; }

    // tell the user how many numbers to enter total
    printf("Enter %d values row by row:\n", rows * cols);

    // loop through every row
    for (int i = 0; i < rows; i++) {
        // loop through every column in this row
        for (int j = 0; j < cols; j++) {
            double val;

            // show the user which cell theyre filling in right now
            // like [0][0] then [0][1] etc so they know where they are
            printf("  [%d][%d]: ", i, j);

            // read a decimal number from the user
            // %lf is the format for double - i accidentally used %f at first
            // which caused weird bugs because %f is for float not double
            scanf("%lf", &val);

            // store the value in the matrix at the right position
            matrix_set(m, i, j, val);
        }
    }

    // return the finished matrix back to main
    return m;
}

// helper that calls whichever operation the user asked for
// returns the result matrix or NULL if the operation name is unknown
// or if the dimensions dont work for that operation
static Matrix *do_operation(const char *op, const Matrix *A, const Matrix *B) {
    if (strcmp(op, "add") == 0) return matrix_add(A, B);
    if (strcmp(op, "sub") == 0) return matrix_sub(A, B);
    if (strcmp(op, "mul") == 0) return matrix_mul(A, B);
    // if we get here the user typed something we dont recognize
    printf("Unknown operation '%s'. Use add, sub, or mul.\n", op);
    return NULL;
}

// main is where the program starts
// argc is how many arguments were passed, argv is the actual argument strings
int main(int argc, char *argv[]) {

    // -------------------------------------------------------
    // FILE MODE - user passed arguments on the command line
    // like: ./matrix_demo mul data/a.txt data/b.txt result.txt
    // -------------------------------------------------------
    if (argc == 4 || argc == 5) {
        const char *op      = argv[1];   // the operation: add, sub, or mul
        const char *file_a  = argv[2];   // path to first matrix file
        const char *file_b  = argv[3];   // path to second matrix file

        // try to load both matrices from their files
        printf("Loading matrix A from '%s'...\n", file_a);
        Matrix *A = matrix_from_file(file_a);

        printf("Loading matrix B from '%s'...\n", file_b);
        Matrix *B = matrix_from_file(file_b);

        // if either file failed to load we cant continue
        if (!A || !B) {
            matrix_free(A);
            matrix_free(B);
            return 1;  // exit with error code
        }

        printf("\nA (%dx%d):\n", A->rows, A->cols);
        matrix_print(A);
        printf("\nB (%dx%d):\n", B->rows, B->cols);
        matrix_print(B);

        // run the operation
        Matrix *result = do_operation(op, A, B);

        if (result) {
            printf("\nResult of %s (%dx%d):\n", op, result->rows, result->cols);
            matrix_print(result);

            // if the user provided an output filename, save to it
            if (argc == 5) {
                const char *outfile = argv[4];
                if (matrix_to_file(result, outfile) == 0)
                    printf("\nResult saved to '%s'\n", outfile);
            }

            matrix_free(result);
        } else {
            // operation returned NULL which means dimensions were incompatible
            printf("Error: incompatible dimensions for '%s'.\n", op);
            printf("For add/sub: both matrices must be the same size.\n");
            printf("For mul: columns of A (%d) must equal rows of B (%d).\n",
                   A->cols, B->rows);
        }

        // always free A and B even if the operation failed
        matrix_free(A);
        matrix_free(B);
        return 0;
    }

    // -------------------------------------------------------
    // INTERACTIVE MODE - no arguments, ask the user to type everything
    // -------------------------------------------------------
    if (argc == 1) {
        // this will hold the operation the user types - max 3 chars plus null terminator
        // "add", "sub", "mul" are all 3 characters long so 4 is enough
        char op[4];

        // ask the user which operation they want to do
        printf("Operation (add / sub / mul): ");

        // read up to 3 characters so we dont overflow the op buffer
        // i learned about buffer overflows and they sound really scary
        scanf("%3s", op);

        // read both matrices from the user by calling our helper function
        Matrix *A = read_matrix_interactive("A");
        Matrix *B = read_matrix_interactive("B");

        // if either matrix failed to load for some reason, stop here
        // returning 1 from main means the program ended with an error
        if (!A || !B) {
            matrix_free(A);
            matrix_free(B);
            return 1;
        }

        // show the user what matrices they entered so they can double check
        printf("\nA:\n"); matrix_print(A);
        printf("B:\n"); matrix_print(B);

        // run the operation and show the result
        Matrix *result = do_operation(op, A, B);

        if (result) {
            printf("\nResult:\n");
            matrix_print(result);
            matrix_free(result);
        } else if (strcmp(op,"add")==0 || strcmp(op,"sub")==0 || strcmp(op,"mul")==0) {
            // result was NULL but op was valid, so dimensions must be wrong
            printf("Error: incompatible dimensions for %s.\n", op);
            printf("For add and sub the matrices must be the same size.\n");
            printf("For mul the columns of A must equal the rows of B.\n");
        }

        // always free A and B at the end to avoid memory leaks
        matrix_free(A);
        matrix_free(B);
        return 0;
    }

    // if we get here the user passed a weird number of arguments
    print_usage(argv[0]);
    return 1;
}
