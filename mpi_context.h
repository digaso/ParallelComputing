#ifndef MPI_CONTEXT_H
#define MPI_CONTEXT_H

#include <mpi.h>

typedef struct {
    int rank;               // Process rank
    int size;               // Total number of processes
    int q;                  // Grid dimension (sqrt(size))
    int grid_rank;          // Rank in grid communicator
    int grid_coords[2];     // Coordinates in grid
    
    MPI_Comm grid_comm;     // 2D grid communicator
    MPI_Comm row_comm;      // Row communicator
    MPI_Comm col_comm;      // Column communicator
} mpi_context_t;

/**
 * Initialize MPI and create process grid topology
 * Returns 1 on success, 0 on failure
 */
int mpi_initialize(mpi_context_t *ctx, int argc, char *argv[]);

/**
 * Validate that the configuration is valid for Fox algorithm
 * Returns 1 if valid, 0 if invalid
 */
int mpi_validate_configuration(mpi_context_t *ctx, int matrix_size);

/**
 * Cleanup MPI resources
 */
void mpi_cleanup(mpi_context_t *ctx);

#endif // MPI_CONTEXT_H