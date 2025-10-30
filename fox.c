#include "fox.h"

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "matrix.h"
#include "mpi/mpi.h"


struct FoxDetails* initFoxDetails(const int Q, const int N, struct EnvData env_data) {
    struct FoxDetails* foxDetails = malloc(sizeof(struct FoxDetails));

    foxDetails->envData = env_data;
    foxDetails->N = N;
    foxDetails->Q = Q;

    return foxDetails;
}

struct FoxMPI* initFoxMPI(struct FoxDetails fox_details) {
    struct FoxMPI* fox_mpi = malloc(sizeof(struct FoxMPI));

    fox_mpi->fox_details = fox_details;
    
    // Initialize MPI objects to safe values
    fox_mpi->cart = MPI_COMM_NULL;
    fox_mpi->row = MPI_COMM_NULL;
    fox_mpi->col = MPI_COMM_NULL;
    fox_mpi->datatype = MPI_DATATYPE_NULL;

    return fox_mpi;
}

int setup_mpi_datatype(MPI_Datatype* datatype, const struct FoxDetails* fox_details) {
    //The size of the matrix per process
    const int perProcessMatrixSize = fox_details->N / fox_details->Q;

    // Create a contiguous datatype for the flat array approach (like Trabalho_1)
    MPI_Type_contiguous(perProcessMatrixSize * perProcessMatrixSize, MATRIX_ELEMENT_MPI, datatype);

    //Let MPI know about the new datatype
    MPI_Type_commit(datatype);

    return 1;
}

int setup_grid(struct FoxMPI* fox_mpi) {
    setup_mpi_datatype(&fox_mpi->datatype, &fox_mpi->fox_details);

    int rank;

    //Get our rank before setting up the grid
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //Setup the dimensions of the grid (We want it to be QxQ)
    const int dimensions[ 2 ] = { fox_mpi->fox_details.Q, fox_mpi->fox_details.Q };

    //Specify that the dimensions are circular, not linear
    const int periods[ 2 ] = { 1, 1 };

    //We want to create a grid with 2 dimensions (Rows and Cols)
    MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, periods, 1, &fox_mpi->cart);

    //Get our new rank in the grid as reorder is set to true
    MPI_Comm_rank(fox_mpi->cart, &rank);

    fox_mpi->fox_details.myRank = rank;

    int coordinates[ 2 ] = { 0 };

    //Get our coordinates inside the grid
    MPI_Cart_coords(fox_mpi->cart, rank, 2,
        coordinates);

    fox_mpi->fox_details.myRow = coordinates[ 0 ];
    fox_mpi->fox_details.myColumn = coordinates[ 1 ];

    //Set up row communicator (We want to vary the column, but not the row)
    const int row_coords[ 2 ] = { 0, 1 };
    MPI_Cart_sub(fox_mpi->cart, row_coords, &fox_mpi->row);

    //Set up column communicator (We want to vary the row, but not the column)
    const int col_coords[ 2 ] = { 1, 0 };
    MPI_Cart_sub(fox_mpi->cart, col_coords, &fox_mpi->col);

    return 1;
}


Matrix* divideMatrix(Matrix matrix, struct GraphData* graphData, struct EnvData* envData) {
    int Q = (int)sqrt(envData->processors);
    int matrixSize = graphData->matrixSize;
    int perProcessSize = matrixSize / Q;
    
    // Allocate array of matrices for each process
    Matrix* processMatrices = malloc(envData->processors * sizeof(Matrix));
    if (!processMatrices) {
        printf("Error: Failed to allocate processMatrices\n");
        return NULL;
    }
    
    // Initialize all pointers to NULL
    for (int proc = 0; proc < envData->processors; proc++) {
        processMatrices[proc] = NULL;
    }
    
    for (int proc = 0; proc < envData->processors; proc++) {
        if (allocate_matrix(perProcessSize, &processMatrices[proc]) != 1) {
            printf("Error: Failed to allocate matrix for process %d\n", proc);
            return NULL;
        }
        
        // Calculate starting position for this process (based on Trabalho_1 approach)
        int startRow = (proc / Q) * perProcessSize;
        int startCol = (proc % Q) * perProcessSize;
        
        // Extract submatrix for this process
        int k = 0;
        for (int i = 0; i < perProcessSize; i++) {
            for (int j = 0; j < perProcessSize; j++) {
                int globalRow = startRow + i;
                int globalCol = startCol + j;
                if (globalRow >= matrixSize || globalCol >= matrixSize) {
                    printf("Error: Index out of bounds - globalRow=%d, globalCol=%d, matrixSize=%d\n", 
                           globalRow, globalCol, matrixSize);
                    return NULL;
                }
                processMatrices[proc][k] = matrix[globalRow * matrixSize + globalCol];
                k++;
            }
        }
    }
    
    return processMatrices;
}

void canRunFox(struct GraphData* graphData, struct EnvData* envData, int* q) {
    int maxQ = (int)(sqrt(envData->processors));
    int possibleProCount = maxQ * maxQ;
    int matrixSize = graphData->matrixSize;


    if (possibleProCount != envData->processors) {
        //The number of processes is not a perfect square.
        fprintf(stderr, "Error: Number of processes (%d) is not a perfect square\n", envData->processors);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (matrixSize % maxQ != 0) {
        fprintf(stderr, "Error: Matrix size (%d) is not divisible by sqrt(processes) (%d)\n", matrixSize, maxQ);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    *q = maxQ;
}

void performFoxAlgorithm(struct FoxMPI* fox_mpi, Matrix localA, Matrix localB, Matrix localC) {
    int per_process_size = fox_mpi->fox_details.N / fox_mpi->fox_details.Q;
    Matrix tempA = NULL;
    allocate_matrix(per_process_size, &tempA);
    
    // Fill tempA with appropriate values
    for (int i = 0; i < per_process_size * per_process_size; i++) {
        tempA[i] = 0;
    }
    
    // Fox algorithm starting
    for (int step = 0; step < fox_mpi->fox_details.Q; step++) {
        // Calculate the root for broadcasting in this step
        int bcast_root = (fox_mpi->fox_details.myRow + step) % fox_mpi->fox_details.Q;
        
        // Broadcasting step
        if (bcast_root == fox_mpi->fox_details.myColumn) {
            // Broadcasting local A matrix
            // Broadcast our localA matrix to processes in our row
            MPI_Bcast(localA, 1, fox_mpi->datatype, bcast_root, fox_mpi->row);
            
            // Perform matrix multiplication with our own matrices
            struct GraphData gd;
            gd.matrixSize = per_process_size;
            multiply_matrix(gd, localA, localB, localC);
        } else {
            // Receive broadcast from the appropriate process in our row
            MPI_Bcast(tempA, 1, fox_mpi->datatype, bcast_root, fox_mpi->row);
            
            // Perform matrix multiplication with received matrix
            struct GraphData gd;
            gd.matrixSize = per_process_size;
            multiply_matrix(gd, tempA, localB, localC);
        }
        
        // Circular shift of B matrices upward in the column
        int source = (fox_mpi->fox_details.myRow + 1) % fox_mpi->fox_details.Q;
        int dest = (fox_mpi->fox_details.myRow + fox_mpi->fox_details.Q - 1) % fox_mpi->fox_details.Q;
        
        // Shifting B matrices
        MPI_Status status;
        MPI_Sendrecv_replace(localB, 1, fox_mpi->datatype, dest, 37, source, 37, 
                            fox_mpi->col, &status);
    }
    
    // Fox algorithm completed
    free_matrix(&tempA);
}

void performAllPairsShortestPath(struct FoxMPI* fox_mpi, Matrix localA, Matrix localB, Matrix localC) {
    int per_process_size = fox_mpi->fox_details.N / fox_mpi->fox_details.Q;
    int m = 1;
    
    // Initialize C matrix with the original distance matrix (not infinity!)
    copy_matrix(per_process_size, localC, localA);
    
    while (m < fox_mpi->fox_details.N - 1) {
        // Perform one iteration of Fox algorithm
        performFoxAlgorithm(fox_mpi, localA, localB, localC);
        
        // Copy result back to A and B for next iteration
        copy_matrix(per_process_size, localA, localC);
        copy_matrix(per_process_size, localB, localC);
        
        m *= 2;
    }
}

int assembleMatrix(int matrixSize, int Q, int processes, Matrix originalMatrix, Matrix destinationMatrix) {
    int perMatrix = matrixSize / Q;
    int k = 0;

    for (int proc = 0; proc < processes; proc++) {
        // Calculate starting position for this process (same as Trabalho_1)
        int startRow = (proc / Q) * perMatrix;
        int startCol = (proc % Q) * perMatrix;

        // Copy submatrix data back to full matrix
        for (int i = 0; i < perMatrix; i++) {
            for (int j = 0; j < perMatrix; j++) {
                int globalRow = startRow + i;
                int globalCol = startCol + j;
                destinationMatrix[globalRow * matrixSize + globalCol] = originalMatrix[k];
                k++;
            }
        }
    }

    return 1;
}

