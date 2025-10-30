#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <limits.h>

#define INF INT_MAX

/**
 * Allocate a 2D matrix
 */
int **allocate_matrix(int rows, int cols);

/**
 * Free a 2D matrix
 */
void free_matrix(int **matrix, int rows);

/**
 * Copy matrix A to matrix B
 */
void copy_matrix(int **src, int **dest, int rows, int cols);

/**
 * Timing utilities
 */
double get_time();

#endif // MATRIX_UTILS_H