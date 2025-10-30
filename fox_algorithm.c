#include "fox_algorithm.h"
#include "matrix_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void fox_run_algorithm(matrix_data_t *data, mpi_context_t *ctx) {
    double start_time, end_time;
    
    // Start timing (excluding I/O)
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = get_time();
    
    // Apply repeated squaring algorithm
    int iterations = (int)ceil(log2(data->n));
    for (int iter = 0; iter < iterations; iter++) {
        if (iter == 0) {
            // First iteration: square the input matrix
            fox_matrix_square(data->local_matrix, data->local_result, data, ctx);
        } else {
            // Subsequent iterations: square the result
            fox_matrix_square(data->local_result, data->local_result, data, ctx);
        }
    }
    
    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = get_time();
    
    // Print timing information
    if (ctx->rank == 0) {
        fprintf(stderr, "Execution time: %.2f ms\n", (end_time - start_time) * 1000);
    }
}

void fox_matrix_square(int **matrix, int **result, matrix_data_t *data, mpi_context_t *ctx) {
    int block_size = data->block_size;
    
    // Allocate temporary matrices for Fox algorithm
    int **A_block = allocate_matrix(block_size, block_size);
    int **B_block = allocate_matrix(block_size, block_size);
    int **temp_result = allocate_matrix(block_size, block_size);
    
    // Initialize result to infinity
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            temp_result[i][j] = INF;
        }
    }
    
    // Fox algorithm main loop
    for (int step = 0; step < ctx->q; step++) {
        // Determine which process in this row should broadcast A
        int broadcast_col = (ctx->grid_coords[0] + step) % ctx->q;
        
        // Copy the appropriate A block
        if (ctx->grid_coords[1] == broadcast_col) {
            copy_matrix(matrix, A_block, block_size, block_size);
        }
        
        // Broadcast A block along the row
        MPI_Bcast(A_block[0], block_size * block_size, MPI_INT, broadcast_col, ctx->row_comm);
        
        // Copy the current B block
        copy_matrix(matrix, B_block, block_size, block_size);
        
        // Perform min-plus multiplication: temp_result = min(temp_result, A_block * B_block)
        int **product = allocate_matrix(block_size, block_size);
        min_plus_multiply(A_block, B_block, product, block_size);
        
        // Update temp_result with minimum values
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                if (product[i][j] < temp_result[i][j]) {
                    temp_result[i][j] = product[i][j];
                }
            }
        }
        
        free_matrix(product, block_size);
        
        // Shift B blocks up in the column (skip for q=1)
        if (ctx->q > 1) {
            int src = (ctx->grid_coords[0] + 1) % ctx->q;
            int dest = (ctx->grid_coords[0] + ctx->q - 1) % ctx->q;
            
            int src_coords[2] = {src, ctx->grid_coords[1]};
            int dest_coords[2] = {dest, ctx->grid_coords[1]};
            
            int src_rank, dest_rank;
            MPI_Cart_rank(ctx->grid_comm, src_coords, &src_rank);
            MPI_Cart_rank(ctx->grid_comm, dest_coords, &dest_rank);
            
            // Use temporary storage for the shift
            int **temp_matrix = allocate_matrix(block_size, block_size);
            
            // Perform the circular shift using MPI_Sendrecv  
            MPI_Sendrecv(matrix[0], block_size * block_size, MPI_INT, dest_rank, 0,
                        temp_matrix[0], block_size * block_size, MPI_INT, src_rank, 0,
                        ctx->grid_comm, MPI_STATUS_IGNORE);
            
            copy_matrix(temp_matrix, matrix, block_size, block_size);
            free_matrix(temp_matrix, block_size);
        }
    }
    
    // Copy final result
    copy_matrix(temp_result, result, block_size, block_size);
    
    // Cleanup
    free_matrix(A_block, block_size);
    free_matrix(B_block, block_size);
    free_matrix(temp_result, block_size);
}

void min_plus_multiply(int **A, int **B, int **C, int block_size) {
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            C[i][j] = INF;
            for (int k = 0; k < block_size; k++) {
                if (A[i][k] != INF && B[k][j] != INF) {
                    long long sum = (long long)A[i][k] + (long long)B[k][j];
                    if (sum < INF && sum < (long long)C[i][j]) {
                        C[i][j] = (int)sum;
                    }
                }
            }
        }
    }
}