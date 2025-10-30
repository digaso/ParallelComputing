#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include "mpi_context.h"
#include "matrix_utils.h"

typedef struct {
    int n;                  // Matrix dimension
    int block_size;         // Block size per process
    int **global_matrix;    // Full matrix (only on rank 0)
    int **result_matrix;    // Result matrix (only on rank 0)
    int **local_matrix;     // Local matrix block
    int **local_result;     // Local result block
} matrix_data_t;

/**
 * Read input matrix and distribute to processes
 * Returns 1 on success, 0 on failure
 */
int io_read_and_distribute(matrix_data_t *data, mpi_context_t *ctx);

/**
 * Gather results from all processes and output
 */
void io_gather_and_output(matrix_data_t *data, mpi_context_t *ctx);

/**
 * Output results and cleanup memory
 */
void io_output_and_cleanup(matrix_data_t *data, mpi_context_t *ctx);

/**
 * Print matrix (only on rank 0)
 */
void print_matrix(int **matrix, int n, int rank);

#endif // IO_HANDLER_H