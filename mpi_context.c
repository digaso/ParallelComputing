#include "mpi_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int mpi_initialize(mpi_context_t *ctx, int argc, char *argv[]) {
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ctx->rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ctx->size);
    
    // Check if number of processes is a perfect square
    ctx->q = (int)sqrt(ctx->size);
    if (ctx->q * ctx->q != ctx->size) {
        if (ctx->rank == 0) {
            fprintf(stderr, "Error: Number of processes (%d) must be a perfect square\n", ctx->size);
        }
        MPI_Finalize();
        return 0;
    }
    
    // Create process grid topology
    int dims[2] = {ctx->q, ctx->q};
    int periods[2] = {1, 1};  // Periodic boundaries
    int reorder = 1;
    
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &ctx->grid_comm);
    
    // Get coordinates in the grid
    MPI_Comm_rank(ctx->grid_comm, &ctx->grid_rank);
    MPI_Cart_coords(ctx->grid_comm, ctx->grid_rank, 2, ctx->grid_coords);
    
    // Create row and column communicators
    int remain_dims[2];
    
    // Row communicator (vary column, fix row)
    remain_dims[0] = 0;
    remain_dims[1] = 1;
    MPI_Cart_sub(ctx->grid_comm, remain_dims, &ctx->row_comm);
    
    // Column communicator (vary row, fix column)
    remain_dims[0] = 1;
    remain_dims[1] = 0;
    MPI_Cart_sub(ctx->grid_comm, remain_dims, &ctx->col_comm);
    
    return 1;
}

int mpi_validate_configuration(mpi_context_t *ctx, int matrix_size) {
    if (matrix_size % ctx->q != 0) {
        if (ctx->rank == 0) {
            fprintf(stderr, "Error: Matrix dimension (%d) must be divisible by sqrt(processes) (%d)\n", 
                   matrix_size, ctx->q);
        }
        return 0;
    }
    return 1;
}

void mpi_cleanup(mpi_context_t *ctx) {
    MPI_Comm_free(&ctx->row_comm);
    MPI_Comm_free(&ctx->col_comm);
    MPI_Comm_free(&ctx->grid_comm);
    MPI_Finalize();
}