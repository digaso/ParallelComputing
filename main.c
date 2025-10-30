#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "mpi/mpi.h"
#include "math.h"
#include "fox.h"


#define ROOT 0

int main(int argc, char** argv) {
    int rank, size, n, q;
    int** global_matrix = NULL;
    int** result_matrix = NULL;
    double start_time, end_time;

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
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);


    // Broadcast matrix dimension to all processes
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Check if Fox's algorithm can be applied
    gd->matrixSize = n;
    ed->processors = size;
    canRunFox(gd, ed);


    return 0;
}