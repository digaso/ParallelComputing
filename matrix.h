#ifndef PARALELCOMPUTING_MATRIX_H
#define PARALELCOMPUTING_MATRIX_H

#include <stdio.h>
#include "io.h"
#include <limits.h>
#include "mpi/mpi.h"

#define MATRIX_ELEMENT_MAX LONG_MAX
#define MATRIX_ELEMENT_MPI MPI_UINT64_T

typedef unsigned long MatrixElement;
typedef MatrixElement* Matrix;

int allocate_matrix(unsigned int size, Matrix* destination);

int copy_matrix(unsigned int size, Matrix destination, Matrix source);

int free_matrix(Matrix* matrix);

int read_matrix(FILE* matrixText, struct GraphData, Matrix matrix);

int write_matrix(FILE* matrixText, struct GraphData, Matrix matrix);

int fill_matrix(struct GraphData, Matrix matrix);

int multiply_matrix(struct GraphData, Matrix matrix_1, Matrix matrix_2, Matrix final_matrix);

int repeated_squaring_algorithm(struct GraphData, Matrix weight_matrix, Matrix result_matrix);

#endif //PARALELCOMPUTING_MATRIX_H