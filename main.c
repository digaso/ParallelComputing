#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "mpi/mpi.h"
#include "math.h"
#include "fox.h"

#define ROOT 0

int main(int argc, char** argv) {
    int rank, size, n, q;
    double start_time, end_time;
    double elapsed_time;

    // Initialize timers

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    start_time = MPI_Wtime();

    FILE* f = stdin;

    // Read matrix dimension
    if (rank == 0) {
        if (fscanf(f, "%d", &n) != 1) {
            fprintf(stderr, "Error reading matrix dimension\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast matrix dimension to all processes
    MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    // ALL processes allocate dividedMatrix (like Trabalho_1)
    Matrix dividedMatrix = NULL;
    allocate_matrix(n, &dividedMatrix);

    if (rank == 0) {
        struct GraphData gd;
        struct EnvData ed;
        gd.matrixSize = n;
        ed.processors = size;

        // Check if Fox's algorithm can be applied
        canRunFox(&gd, &ed, &q);

        // Allocate and read the full matrix
        Matrix matrix = NULL;
        allocate_matrix(n, &matrix);

        if (read_matrix(f, gd, matrix) != 1) {
            fprintf(stderr, "Error reading matrix data\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fill_matrix(gd, matrix);

        // Build scatter matrix using our divideMatrix function
        Matrix* temp_divided = divideMatrix(matrix, &gd, &ed);
        if (!temp_divided) {
            fprintf(stderr, "Error: Failed to divide matrix\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Copy to dividedMatrix in the correct order (like buildScatterMatrix from Trabalho_1)
        int k = 0;
        int per_process_size = n / q;

        // Matrix divided into submatrices for each process
        for (int proc = 0; proc < size; proc++) {
            for (int i = 0; i < per_process_size * per_process_size; i++) {
                dividedMatrix[ k++ ] = temp_divided[ proc ][ i ];
            }
        }

        // Clean up
        for (int i = 0; i < size; i++) {
            free_matrix(&temp_divided[ i ]);
        }
        free(temp_divided);
        free_matrix(&matrix);
    }

    // Broadcast Q to all processes
    MPI_Bcast(&q, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Setup Fox structures
    struct EnvData ed;
    ed.processors = size;
    struct FoxDetails* fox_details = initFoxDetails(q, n, ed);
    struct FoxMPI* fox_mpi = initFoxMPI(*fox_details);

    // Setup MPI grid and datatype
    setup_grid(fox_mpi);

    // Allocate local matrices
    Matrix localA = NULL, localB = NULL, localC = NULL;
    int per_process_size = n / q;
    allocate_matrix(per_process_size, &localA);
    allocate_matrix(per_process_size, &localB);
    allocate_matrix(per_process_size, &localC);

    // Scatter 
    MPI_Scatter(dividedMatrix, 1, fox_mpi->datatype, localA, 1, fox_mpi->datatype, ROOT, fox_mpi->cart);

    // Copy A to B initially
    copy_matrix(per_process_size, localB, localA);

    // Execute Fox algorithm for all-pairs shortest path
    performAllPairsShortestPath(fox_mpi, localA, localB, localC);

    // Gather results 
    MPI_Gather(localC, 1, fox_mpi->datatype, dividedMatrix, 1, fox_mpi->datatype, ROOT, fox_mpi->cart);

    if (fox_mpi->fox_details.myRank == ROOT) {
        Matrix finalDestination = NULL;
        allocate_matrix(n, &finalDestination);

        assembleMatrix(n, q, size, dividedMatrix, finalDestination);

        printf("\nFinal shortest path matrix:\n");
        struct GraphData gd;
        gd.matrixSize = n;
        write_matrix(stdout, gd, finalDestination);

        free_matrix(&finalDestination);
    }

    // Cleanup
    free_matrix(&dividedMatrix);
    free_matrix(&localA);
    free_matrix(&localB);
    free_matrix(&localC);
    free(fox_details);
    free_fox_mpi(fox_mpi);
    end_time = MPI_Wtime();
    elapsed_time = end_time - start_time;
    if (rank == 0) {
        printf("Total execution time: %f seconds\n", elapsed_time);
    }
    MPI_Finalize();
    return 0;
}