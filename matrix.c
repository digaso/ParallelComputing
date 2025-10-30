#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fox.h"


int allocate_matrix(unsigned int size, Matrix* destination) {
    if (*destination != NULL) {
        return 0;
    }

    *destination = (Matrix)malloc(size * size * sizeof(MatrixElement));

    return 1;
}

int copy_matrix(unsigned int size, Matrix destination, Matrix source) {

    if (destination == NULL || source == NULL)
        return 0;

    memcpy(destination, source, size * size * sizeof(MatrixElement));
    return 1;
}

int free_matrix(Matrix* matrix) {
    if (matrix != NULL) {
        free(*matrix);

        *matrix = NULL;
        return 1;
    }

    return 0;
}

int read_matrix(FILE* matrixText, struct GraphData graph_data, Matrix matrix) {

    for (int row = 0; row < graph_data.matrixSize; row++) {

        for (int column = 0; column < graph_data.matrixSize; column++) {
            fscanf(matrixText, "%ld", &matrix[ calculateProjection(graph_data.matrixSize, row, column) ]);
        }
    }

    return 1;
}

int write_matrix(FILE* matrixText, struct GraphData graph_data, Matrix matrix) {
    for (int row = 0; row < graph_data.matrixSize; row++) {
        int first = 1;

        for (int column = 0; column < graph_data.matrixSize; column++) {
            int pos = calculateProjection(graph_data.matrixSize, row, column);

            if (first) {
                first = 0;
            }
            else {
                fprintf(matrixText, " ");
            }

            if (matrix[ pos ] >= MATRIX_ELEMENT_MAX - 2) {
                fprintf(matrixText, "0");  // Output 0 for unreachable paths instead of IM
            }
            else {
                fprintf(matrixText, "%ld", matrix[ pos ]);
            }
        }

        fprintf(matrixText, "\n");
    }
    return 1;
}

int fill_matrix(struct GraphData graph_data, Matrix matrix) {
    const int matrixSize = graph_data.matrixSize;

    // Filling matrix with infinity values
    
    if (!matrix) {
        printf("Error: Matrix is NULL in fill_matrix\n");
        return 0;
    }

    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            int index = calculateProjection(matrixSize, i, j);
            if (index < 0 || index >= matrixSize * matrixSize) {
                printf("Error: Index out of bounds in fill_matrix - index=%d, max=%d\n", 
                       index, matrixSize * matrixSize);
                return 0;
            }
            if (i != j && matrix[index] == 0) {
                matrix[index] = MATRIX_ELEMENT_MAX - 1;
            }
        }
    }

    // Matrix fill completed
    return 1;
}

MatrixElement min(MatrixElement a, MatrixElement b) {
    return a < b ? a : b;
}

int multiply_matrix(const struct GraphData graph_data, Matrix matrix_1, Matrix matrix_2, Matrix final_matrix) {
    const int matrixSize = graph_data.matrixSize;

    for (int i = 0; i < matrixSize; i++) {

        for (int j = 0; j < matrixSize; j++) {

            for (int k = 0; k < matrixSize; k++) {
                const MatrixElement current_value = final_matrix[ calculateProjection(matrixSize, i, j) ];

                const MatrixElement new_value = matrix_1[ calculateProjection(matrixSize, i, k) ] + matrix_2[ calculateProjection(matrixSize, k, j) ];

                final_matrix[ calculateProjection(matrixSize, i, j) ] = min(current_value, new_value);
            }
        }
    }
    
    return 1;
}

int repeated_squaring_algorithm(const struct GraphData graph_data, Matrix weight_matrix, Matrix result_matrix) {
    const int matrixSize = graph_data.matrixSize;

    int m = 1;

    fill_matrix(graph_data, result_matrix);

    while (m < matrixSize - 1) {
        multiply_matrix(graph_data, weight_matrix, weight_matrix, result_matrix);

        m *= 2;

        copy_matrix(matrixSize, weight_matrix, result_matrix);
    }

    return 1;
}