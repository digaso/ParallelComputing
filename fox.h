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

struct FoxMPI;

struct FoxDetails* initFoxDetails(int Q, int N, struct EnvData env_data);

struct FoxMPI* initFoxMPI(struct FoxDetails fox_details);

inline int calculateStartRow(const int processID, const struct FoxDetails* foxDetails) {
    return processID / foxDetails->Q * foxDetails->per_process_n;
}

inline int calculateStartColumn(const int processID, const struct FoxDetails* foxDetails) {
    return processID % foxDetails->Q * foxDetails->per_process_n;
}

inline int calculateProjection(const int size, const int row, const int column) {
    return row * size + column;
}

void canRunFox(struct GraphData* graphData, struct EnvData* envData);

Matrix* divideMatrix(Matrix matrix, struct EnvData* envData);


#endif //PARALELCOMPUTING_FOX_H