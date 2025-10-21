#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <limits.h>
#include <sys/time.h>

#define INF INT_MAX

// Function prototypes
void read_input_matrix(int **matrix, int n, int rank);
void print_matrix(int **matrix, int n, int rank);
int **allocate_matrix(int rows, int cols);
void free_matrix(int **matrix, int rows);
void fox_algorithm(int **A, int **B, int **C, int n, int q, MPI_Comm grid_comm, 
                   MPI_Comm row_comm, MPI_Comm col_comm, int grid_rank, int grid_coords[2]);
void min_plus_multiply(int **A, int **B, int **C, int block_size);
void min_plus_square(int **matrix, int **result, int n, int q, MPI_Comm grid_comm,
                     MPI_Comm row_comm, MPI_Comm col_comm, int grid_rank, int grid_coords[2]);
double get_time();

int main(int argc, char *argv[]) {
    int rank, size, n, q;
    int **global_matrix = NULL;
    int **result_matrix = NULL;
    double start_time, end_time;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Read matrix dimension
    if (rank == 0) {
        if (scanf("%d", &n) != 1) {
            fprintf(stderr, "Error reading matrix dimension\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Broadcast matrix dimension to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Check if Fox's algorithm can be applied
    q = (int)sqrt(size);
    if (q * q != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Number of processes (%d) must be a perfect square\n", size);
        }
        MPI_Finalize();
        return 1;
    }
    
    if (n % q != 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: Matrix dimension (%d) must be divisible by sqrt(processes) (%d)\n", n, q);
        }
        MPI_Finalize();
        return 1;
    }
    
    // Allocate memory for global matrix on process 0
    if (rank == 0) {
        global_matrix = allocate_matrix(n, n);
        result_matrix = allocate_matrix(n, n);
    }
    
    // Read input matrix
    read_input_matrix(global_matrix, n, rank);
    
    // Start timing (excluding I/O)
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = get_time();
    
    // Create process grid topology
    int dims[2] = {q, q};
    int periods[2] = {1, 1};  // Periodic boundaries
    int reorder = 1;
    MPI_Comm grid_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &grid_comm);
    
    // Get coordinates in the grid
    int grid_rank, grid_coords[2];
    MPI_Comm_rank(grid_comm, &grid_rank);
    MPI_Cart_coords(grid_comm, grid_rank, 2, grid_coords);
    
    // Create row and column communicators
    MPI_Comm row_comm, col_comm;
    int remain_dims[2];
    
    // Row communicator (vary column, fix row)
    remain_dims[0] = 0;
    remain_dims[1] = 1;
    MPI_Cart_sub(grid_comm, remain_dims, &row_comm);
    
    // Column communicator (vary row, fix column)
    remain_dims[0] = 1;
    remain_dims[1] = 0;
    MPI_Cart_sub(grid_comm, remain_dims, &col_comm);
    
    // Calculate block size
    int block_size = n / q;
    
    // Allocate local matrices
    int **local_A = allocate_matrix(block_size, block_size);
    int **local_result = allocate_matrix(block_size, block_size);
    
    // Distribute initial matrix to all processes
    if (rank == 0) {
        // Send blocks to appropriate processes
        for (int proc_row = 0; proc_row < q; proc_row++) {
            for (int proc_col = 0; proc_col < q; proc_col++) {
                int dest_coords[2] = {proc_row, proc_col};
                int dest_rank;
                MPI_Cart_rank(grid_comm, dest_coords, &dest_rank);
                
                // Copy block data
                int **block = allocate_matrix(block_size, block_size);
                for (int i = 0; i < block_size; i++) {
                    for (int j = 0; j < block_size; j++) {
                        block[i][j] = global_matrix[proc_row * block_size + i][proc_col * block_size + j];
                    }
                }
                
                if (dest_rank == 0) {
                    // Copy to local matrix for process 0
                    for (int i = 0; i < block_size; i++) {
                        for (int j = 0; j < block_size; j++) {
                            local_A[i][j] = block[i][j];
                        }
                    }
                } else {
                    // Send to other processes
                    for (int i = 0; i < block_size; i++) {
                        MPI_Send(block[i], block_size, MPI_INT, dest_rank, 0, grid_comm);
                    }
                }
                free_matrix(block, block_size);
            }
        }
    } else {
        // Receive block from process 0
        for (int i = 0; i < block_size; i++) {
            MPI_Recv(local_A[i], block_size, MPI_INT, 0, 0, grid_comm, MPI_STATUS_IGNORE);
        }
    }
    
    // Initialize result matrix as copy of input matrix
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            local_result[i][j] = local_A[i][j];
        }
    }
    
    // Apply repeated squaring algorithm
    int iterations = (int)ceil(log2(n));
    for (int iter = 0; iter < iterations; iter++) {
        min_plus_square(local_result, local_result, n, q, grid_comm, row_comm, col_comm, grid_rank, grid_coords);
    }
    
    // Gather results back to process 0
    if (rank == 0) {
        // Copy local result for process 0
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                result_matrix[i][j] = local_result[i][j];
            }
        }
        
        // Receive blocks from other processes
        for (int proc_row = 0; proc_row < q; proc_row++) {
            for (int proc_col = 0; proc_col < q; proc_col++) {
                if (proc_row == 0 && proc_col == 0) continue; // Skip process 0
                
                int src_coords[2] = {proc_row, proc_col};
                int src_rank;
                MPI_Cart_rank(grid_comm, src_coords, &src_rank);
                
                int **block = allocate_matrix(block_size, block_size);
                for (int i = 0; i < block_size; i++) {
                    MPI_Recv(block[i], block_size, MPI_INT, src_rank, 1, grid_comm, MPI_STATUS_IGNORE);
                }
                
                // Copy to result matrix
                for (int i = 0; i < block_size; i++) {
                    for (int j = 0; j < block_size; j++) {
                        result_matrix[proc_row * block_size + i][proc_col * block_size + j] = block[i][j];
                    }
                }
                free_matrix(block, block_size);
            }
        }
    } else {
        // Send local result to process 0
        for (int i = 0; i < block_size; i++) {
            MPI_Send(local_result[i], block_size, MPI_INT, 0, 1, grid_comm);
        }
    }
    
    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = get_time();
    
    // Print results and timing information
    if (rank == 0) {
        print_matrix(result_matrix, n, rank);
        fprintf(stderr, "Execution time: %.2f ms\n", (end_time - start_time) * 1000);
    }
    
    // Cleanup
    free_matrix(local_A, block_size);
    free_matrix(local_result, block_size);
    if (rank == 0) {
        free_matrix(global_matrix, n);
        free_matrix(result_matrix, n);
    }
    
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
    MPI_Comm_free(&grid_comm);
    MPI_Finalize();
    
    return 0;
}

void read_input_matrix(int **matrix, int n, int rank) {
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

int **allocate_matrix(int rows, int cols) {
    int **matrix = (int **)malloc(rows * sizeof(int *));
    if (matrix == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc(cols * sizeof(int));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    return matrix;
}

void free_matrix(int **matrix, int rows) {
    if (matrix != NULL) {
        for (int i = 0; i < rows; i++) {
            free(matrix[i]);
        }
        free(matrix);
    }
}

void min_plus_multiply(int **A, int **B, int **C, int block_size) {
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            C[i][j] = INF;
            for (int k = 0; k < block_size; k++) {
                if (A[i][k] != INF && B[k][j] != INF) {
                    int sum = A[i][k] + B[k][j];
                    if (sum < C[i][j]) {
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
}

void min_plus_square(int **matrix, int **result, int n, int q, MPI_Comm grid_comm,
                     MPI_Comm row_comm, MPI_Comm col_comm, int grid_rank, int grid_coords[2]) {
    int block_size = n / q;
    int **temp_A = allocate_matrix(block_size, block_size);
    int **temp_B = allocate_matrix(block_size, block_size);
    int **temp_C = allocate_matrix(block_size, block_size);
    
    // Initialize temp_C to INF
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            temp_C[i][j] = INF;
        }
    }
    
    // Fox's algorithm for min-plus matrix multiplication
    for (int stage = 0; stage < q; stage++) {
        // Determine which process in this row should broadcast
        int bcast_col = (grid_coords[0] + stage) % q;
        
        // Copy appropriate block to temp_A
        if (grid_coords[1] == bcast_col) {
            // This process has the block to broadcast
            for (int i = 0; i < block_size; i++) {
                for (int j = 0; j < block_size; j++) {
                    temp_A[i][j] = matrix[i][j];
                }
            }
        }
        
        // Broadcast block A within row
        int bcast_rank;
        int bcast_coords[2] = {grid_coords[0], bcast_col};
        MPI_Cart_rank(grid_comm, bcast_coords, &bcast_rank);
        
        for (int i = 0; i < block_size; i++) {
            MPI_Bcast(temp_A[i], block_size, MPI_INT, bcast_col, row_comm);
        }
        
        // Copy current block to temp_B
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                temp_B[i][j] = matrix[i][j];
            }
        }
        
        // Perform min-plus multiplication and accumulate result
        int **stage_result = allocate_matrix(block_size, block_size);
        min_plus_multiply(temp_A, temp_B, stage_result, block_size);
        
        // Take minimum with current result
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                if (stage_result[i][j] < temp_C[i][j]) {
                    temp_C[i][j] = stage_result[i][j];
                }
            }
        }
        
        free_matrix(stage_result, block_size);
        
        // Shift matrix B up within column (circular shift)
        int up_rank, down_rank;
        MPI_Cart_shift(grid_comm, 0, -1, &down_rank, &up_rank);
        
        // Send current block up and receive new block from below
        int **new_B = allocate_matrix(block_size, block_size);
        for (int i = 0; i < block_size; i++) {
            MPI_Sendrecv(matrix[i], block_size, MPI_INT, up_rank, 0,
                        new_B[i], block_size, MPI_INT, down_rank, 0,
                        grid_comm, MPI_STATUS_IGNORE);
        }
        
        // Copy new block back to matrix
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                matrix[i][j] = new_B[i][j];
            }
        }
        free_matrix(new_B, block_size);
    }
    
    // Copy result back
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            result[i][j] = temp_C[i][j];
        }
    }
    
    free_matrix(temp_A, block_size);
    free_matrix(temp_B, block_size);
    free_matrix(temp_C, block_size);
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}