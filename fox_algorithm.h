#ifndef FOX_ALGORITHM_H
#define FOX_ALGORITHM_H

#include "mpi_context.h"
#include "io_handler.h"

/**
 * Run the complete Fox algorithm for all-pairs shortest path
 * Includes timing measurement
 */
void fox_run_algorithm(matrix_data_t *data, mpi_context_t *ctx);

/**
 * Perform one iteration of matrix squaring using Fox's algorithm
 */
void fox_matrix_square(int **matrix, int **result, matrix_data_t *data, mpi_context_t *ctx);

/**
 * Min-plus multiplication of two matrix blocks
 */
void min_plus_multiply(int **A, int **B, int **C, int block_size);

#endif // FOX_ALGORITHM_H