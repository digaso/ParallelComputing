#include "io_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

static void read_input_matrix(int **matrix, int n, int rank) {
    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (scanf("%d", &matrix[i][j]) != 1) {
                    fprintf(stderr, "Error reading matrix element [%d][%d]\n", i, j);
                    MPI_Abort(MPI_COMM_WORLD, 1);
                }
                // Convert 0 to INF for non-diagonal elements (no direct path)
                if (i != j && matrix[i][j] == 0) {
                    matrix[i][j] = INF;
                }
            }
        }
    }
}

static void distribute_matrix_blocks(matrix_data_t *data, mpi_context_t *ctx) {
    if (ctx->rank == 0) {
        // Send blocks to appropriate processes
        for (int proc_row = 0; proc_row < ctx->q; proc_row++) {
            for (int proc_col = 0; proc_col < ctx->q; proc_col++) {
                int dest_coords[2] = {proc_row, proc_col};
                int dest_rank;
                MPI_Cart_rank(ctx->grid_comm, dest_coords, &dest_rank);
                
                // Copy block data
                int **block = allocate_matrix(data->block_size, data->block_size);
                for (int i = 0; i < data->block_size; i++) {
                    for (int j = 0; j < data->block_size; j++) {
                        block[i][j] = data->global_matrix[proc_row * data->block_size + i]
                                                        [proc_col * data->block_size + j];
                    }
                }
                
                if (dest_rank == 0) {
                    // Copy to local matrix for process 0
                    copy_matrix(block, data->local_matrix, data->block_size, data->block_size);
                } else {
                    // Send to other processes
                    for (int i = 0; i < data->block_size; i++) {
                        MPI_Send(block[i], data->block_size, MPI_INT, dest_rank, 0, ctx->grid_comm);
                    }
                }
                free_matrix(block, data->block_size);
            }
        }
    } else {
        // Receive block from process 0
        for (int i = 0; i < data->block_size; i++) {
            MPI_Recv(data->local_matrix[i], data->block_size, MPI_INT, 0, 0, 
                    ctx->grid_comm, MPI_STATUS_IGNORE);
        }
    }
}

int io_read_and_distribute(matrix_data_t *data, mpi_context_t *ctx) {
    // Read matrix dimension
    if (ctx->rank == 0) {
        if (scanf("%d", &data->n) != 1) {
            fprintf(stderr, "Error reading matrix dimension\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Broadcast matrix dimension to all processes
    MPI_Bcast(&data->n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Validate configuration
    if (!mpi_validate_configuration(ctx, data->n)) {
        return 0;
    }
    
    // Calculate block size
    data->block_size = data->n / ctx->q;
    
    // Allocate memory for global matrix on process 0
    if (ctx->rank == 0) {
        data->global_matrix = allocate_matrix(data->n, data->n);
        data->result_matrix = allocate_matrix(data->n, data->n);
    } else {
        data->global_matrix = NULL;
        data->result_matrix = NULL;
    }
    
    // Allocate local matrices for all processes
    data->local_matrix = allocate_matrix(data->block_size, data->block_size);
    data->local_result = allocate_matrix(data->block_size, data->block_size);
    
    // Read input matrix
    read_input_matrix(data->global_matrix, data->n, ctx->rank);
    
    // Distribute matrix blocks to all processes
    distribute_matrix_blocks(data, ctx);
    
    // Initialize result matrix as copy of input matrix
    copy_matrix(data->local_matrix, data->local_result, data->block_size, data->block_size);
    
    return 1;
}

void io_gather_and_output(matrix_data_t *data, mpi_context_t *ctx) {
    if (ctx->rank == 0) {
        // Copy local result for process 0
        for (int i = 0; i < data->block_size; i++) {
            for (int j = 0; j < data->block_size; j++) {
                data->result_matrix[i][j] = data->local_result[i][j];
            }
        }
        
        // Receive blocks from other processes
        for (int proc_row = 0; proc_row < ctx->q; proc_row++) {
            for (int proc_col = 0; proc_col < ctx->q; proc_col++) {
                if (proc_row == 0 && proc_col == 0) continue; // Skip process 0
                
                int src_coords[2] = {proc_row, proc_col};
                int src_rank;
                MPI_Cart_rank(ctx->grid_comm, src_coords, &src_rank);
                
                int **block = allocate_matrix(data->block_size, data->block_size);
                for (int i = 0; i < data->block_size; i++) {
                    MPI_Recv(block[i], data->block_size, MPI_INT, src_rank, 1, 
                            ctx->grid_comm, MPI_STATUS_IGNORE);
                }
                
                // Copy to result matrix
                for (int i = 0; i < data->block_size; i++) {
                    for (int j = 0; j < data->block_size; j++) {
                        data->result_matrix[proc_row * data->block_size + i]
                                          [proc_col * data->block_size + j] = block[i][j];
                    }
                }
                free_matrix(block, data->block_size);
            }
        }
    } else {
        // Send local result to process 0
        for (int i = 0; i < data->block_size; i++) {
            MPI_Send(data->local_result[i], data->block_size, MPI_INT, 0, 1, ctx->grid_comm);
        }
    }
}

void io_output_and_cleanup(matrix_data_t *data, mpi_context_t *ctx) {
    // Gather results
    io_gather_and_output(data, ctx);
    
    // Print results
    if (ctx->rank == 0) {
        print_matrix(data->result_matrix, data->n, ctx->rank);
    }
    
    // Cleanup memory
    free_matrix(data->local_matrix, data->block_size);
    free_matrix(data->local_result, data->block_size);
    
    if (ctx->rank == 0) {
        free_matrix(data->global_matrix, data->n);
        free_matrix(data->result_matrix, data->n);
    }
}

void print_matrix(int **matrix, int n, int rank) {
    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (matrix[i][j] == INF) {
                    printf("0");
                } else {
                    printf("%d", matrix[i][j]);
                }
                if (j < n - 1) printf(" ");
            }
            printf("\n");
        }
    }
}