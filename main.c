#include <stdio.h>
#include <stdlib.h>
#include "mpi_context.h"
#include "io_handler.h"
#include "fox_algorithm.h"

/**
 * Clean, simple main function for Fox's Algorithm All-Pairs Shortest Path
 * 
 * Flow:
 * 1. Initialize MPI and validate configuration
 * 2. Read input and distribute matrix blocks
 * 3. Run Fox algorithm with timing
 * 4. Gather results and output
 * 5. Cleanup and finalize
 */
int main(int argc, char *argv[]) {
    mpi_context_t mpi_ctx;
    matrix_data_t matrix_data;
    
    // 1. Initialize MPI and validate
    if (!mpi_initialize(&mpi_ctx, argc, argv)) {
        return 1;
    }
    
    // 2. Handle input/output and distribution
    if (!io_read_and_distribute(&matrix_data, &mpi_ctx)) {
        mpi_cleanup(&mpi_ctx);
        return 1;
    }
    
    // 3. Run Fox algorithm  
    fox_run_algorithm(&matrix_data, &mpi_ctx);
    
    // 4. Output results and cleanup
    io_output_and_cleanup(&matrix_data, &mpi_ctx);
    mpi_cleanup(&mpi_ctx);
    
    return 0;
}