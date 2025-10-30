#include "fox.h"

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "matrix.h"
#include "mpi/mpi.h"

struct FoxMPI {
    struct FoxDetails fox_details;

    MPI_Comm cart, row, col;

    MPI_Datatype datatype;
};

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

    return fox_mpi;
}

int setup_mpi_datatype(MPI_Datatype* datatype, const struct FoxDetails* fox_details) {
    //The size of the matrix per process
    const int perProcessMatrixSize = fox_details->N / fox_details->Q;

    MPI_Type_vector(perProcessMatrixSize, perProcessMatrixSize, perProcessMatrixSize,
        MATRIX_ELEMENT_MPI, datatype);

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


Matrix* divideMatrix(Matrix matrix, struct EnvData* envData) {


}

void canRunFox(struct GraphData* graphData, struct EnvData* envData, int* q) {
    int maxQ = (int)(sqrt(envData->processors));
    int possibleProCount = maxQ * maxQ;
    int matrixSize = graphData->matrixSize;

    if (possibleProCount != envData->processors) {

        //The number of processes is not a perfect square.

        return 0;
    }

    if (matrixSize % maxQ == 0) {

        *q = maxQ;

        return 1;
    }

    return 0;

}

