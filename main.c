#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "mpi/mpi.h"
#include "math.h"
#include "fox.h"

#define ROOT 0

int main(int argc, char** argv) {
    int rank, size, n, q;
    Matrix global_matrix = NULL;
    Matrix localA = NULL, localB = NULL, localC = NULL;
    
    struct GraphData* gd = malloc(sizeof(struct GraphData));
    struct EnvData* ed = malloc(sizeof(struct EnvData));

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    FILE* f = stdin;

    // Read matrix dimension
    if (rank == 0) {
        if (fscanf(f, "%d", &n) != 1) {
            fprintf(stderr, "Error reading matrix dimension\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast matrix dimension to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Process %d: Matrix dimension: %d\n", rank, n);

    // Setup data structures
    gd->matrixSize = n;
    ed->processors = size;
    
    // Check if Fox's algorithm can be applied
    canRunFox(gd, ed, &q);

    // Initialize Fox algorithm structures
    struct FoxDetails* fox_details = initFoxDetails(q, n, *ed);
    struct FoxMPI* fox_mpi = initFoxMPI(*fox_details);
    
    // Setup MPI grid and datatype
    setup_grid(fox_mpi);
    
    int per_process_size = n / q;
    
    // Allocate local matrices for each process
    allocate_matrix(per_process_size, &localA);
    allocate_matrix(per_process_size, &localB);
    allocate_matrix(per_process_size, &localC);

    // Root process reads the matrix and distributes it
    if (rank == 0) {
        // Allocate and read the full matrix
        allocate_matrix(n, &global_matrix);
        
        if (read_matrix(f, *gd, global_matrix) != 1) {
            fprintf(stderr, "Error reading matrix data\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        printf("Matrix read successfully by root process\n");
        
        // Prepare matrix for all-pairs shortest path algorithm
        printf("Debug: About to fill matrix\n");
        fill_matrix(*gd, global_matrix);
        printf("Debug: Matrix filled, about to divide\n");
        
        // Divide matrix for distribution
        Matrix* divided_matrices = divideMatrix(global_matrix, gd, ed);
        if (!divided_matrices) {
            fprintf(stderr, "Error: Failed to divide matrix\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("Debug: Matrix divided successfully\n");
        
        // Scatter the divided matrices to all processes
        for (int proc = 0; proc < size; proc++) {
            if (proc == 0) {
                // Copy to root's local matrix
                copy_matrix(per_process_size, localA, divided_matrices[proc]);
            } else {
                // Send to other processes (simplified - should use MPI_Scatter)
                MPI_Send(divided_matrices[proc], per_process_size * per_process_size, 
                        MATRIX_ELEMENT_MPI, proc, 0, MPI_COMM_WORLD);
            }
        }
        
        // Clean up divided matrices
        for (int i = 0; i < size; i++) {
            free_matrix(&divided_matrices[i]);
        }
        free(divided_matrices);
        free_matrix(&global_matrix);
    } else {
        // Non-root processes receive their submatrix
        MPI_Recv(localA, per_process_size * per_process_size, MATRIX_ELEMENT_MPI, 
                0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    // Copy A to B initially (for all-pairs shortest path)
    copy_matrix(per_process_size, localB, localA);
    
    // Initialize result matrix C
    fill_matrix(*gd, localC);
    
    printf("Process %d: Local matrices initialized\n", rank);
    
    // Execute Fox algorithm for all-pairs shortest path
    performAllPairsShortestPath(fox_mpi, localA, localB, localC);
    
    printf("Process %d: Fox algorithm completed\n", rank);

    // Clean up MPI structures - let MPI_Finalize handle communicator cleanup
    if (fox_mpi) {
        free(fox_mpi);
    }
    
    // Clean up matrices
    if (localA) free_matrix(&localA);
    if (localB) free_matrix(&localB);
    if (localC) free_matrix(&localC);
    
    // Clean up other structures
    if (fox_details) free(fox_details);
    if (gd) free(gd);
    if (ed) free(ed);
    
    printf("Process %d: Cleanup completed, finalizing MPI\n", rank);
    MPI_Finalize();
    return 0;
}