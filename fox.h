#ifndef PARALELCOMPUTING_FOX_H
#define PARALELCOMPUTING_FOX_H

#include "io.h"
#include "matrix.h"

struct FoxDetails {
    int Q;
    int N;
    int per_process_n;
    int myRow, myColumn, myRank;
    struct EnvData envData;
};

struct FoxMPI {
    struct FoxDetails fox_details;

    MPI_Comm cart, row, col;

    MPI_Datatype datatype;
};

struct FoxDetails* initFoxDetails(int Q, int N, struct EnvData env_data);

struct FoxMPI* initFoxMPI(struct FoxDetails fox_details);

int setup_grid(struct FoxMPI* fox_mpi);

inline int calculateStartRow(const int processID, const struct FoxDetails* foxDetails) {
    return processID / foxDetails->Q * foxDetails->per_process_n;
}

inline int calculateStartColumn(const int processID, const struct FoxDetails* foxDetails) {
    return processID % foxDetails->Q * foxDetails->per_process_n;
}

inline int calculateProjection(const int size, const int row, const int column) {
    return row * size + column;
}

void canRunFox(struct GraphData* graphData, struct EnvData* envData, int* q);

Matrix* divideMatrix(Matrix matrix, struct GraphData* graphData, struct EnvData* envData);

void performFoxAlgorithm(struct FoxMPI* fox_mpi, Matrix localA, Matrix localB, Matrix localC);

void performAllPairsShortestPath(struct FoxMPI* fox_mpi, Matrix localA, Matrix localB, Matrix localC);


#endif //PARALELCOMPUTING_FOX_H