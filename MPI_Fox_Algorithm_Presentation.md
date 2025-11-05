# MPI Fox Algorithm for All-Pairs Shortest Path
## Comprehensive Data Flow and Implementation Analysis

---

## Slide 1: Overview
**Problem:** Compute shortest paths between all pairs of vertices in a graph
**Solution:** Parallel Fox Algorithm using MPI
**Key Insight:** Transform matrix multiplication into distributed computation

---

## Slide 2: Sequential vs Parallel Approach

### Sequential (Floyd-Warshall)
```c
for (k = 0; k < n; k++)
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
```
**Time:** O(N³), **Space:** O(N²)

### Our Parallel Approach
- **Matrix multiplication** using **repeated squaring**
- **Fox algorithm** for parallel matrix multiplication
- **MPI communication** for distributed processing
- **Time:** O(N³/P + communication), **Space:** O(N²/P) per process

---

## Slide 3: Complete Data Flow Overview

```
[Input Matrix] → [Root Process] → [Divide] → [Scatter] → [Parallel Fox] → [Gather] → [Assemble] → [Output]
```

**Processes:** P = Q² (must be perfect square: 4, 9, 16, 25...)
**Matrix Size:** N×N (must be divisible by Q = √P)
**Each Process:** Handles (N/Q)×(N/Q) submatrix

---

## Slide 4: Phase 1 - Input and Validation

### Step 1.1: Input Reading (`main.c:24-35`)
```c
if (rank == 0) {
    fscanf(f, "%d", &n);                    // Read matrix size
}
MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);  // Broadcast to all
```

### Step 1.2: Validation (`fox.c:147-165`)
```c
void canRunFox(struct GraphData* graphData, struct EnvData* envData, int* q) {
    int maxQ = (int)(sqrt(envData->processors));     // Q = √P
    
    if (maxQ * maxQ != envData->processors) {
        // ERROR: Must be perfect square
    }
    if (matrixSize % maxQ != 0) {
        // ERROR: Matrix size must be divisible by Q
    }
}
```

**Example:** 16 processes → Q=4, Matrix 12×12 ✓ (divisible by 4)

---

## Slide 5: Phase 2 - Matrix Division Strategy

### Step 2.1: Full Matrix Reading (Root Only)
```c
Matrix matrix = NULL;
allocate_matrix(n, &matrix);
read_matrix(f, gd, matrix);              // Read full N×N matrix
fill_matrix(gd, matrix);                 // Fill unreachable paths with ∞
```

### Step 2.2: Matrix Division Algorithm (`fox.c:100-145`)
```c
Matrix* divideMatrix(Matrix matrix, struct GraphData* graphData, struct EnvData* envData) {
    int Q = (int)sqrt(envData->processors);
    int perProcessSize = matrixSize / Q;
    
    for (int proc = 0; proc < envData->processors; proc++) {
        int startRow = (proc / Q) * perProcessSize;  // Block row
        int startCol = (proc % Q) * perProcessSize;  // Block column
        
        // Extract (N/Q)×(N/Q) submatrix for this process
        int k = 0;
        for (int i = 0; i < perProcessSize; i++) {
            for (int j = 0; j < perProcessSize; j++) {
                processMatrices[proc][k] = matrix[globalRow * matrixSize + globalCol];
                k++;
            }
        }
    }
}
```

---

## Slide 6: Matrix Division Visual Example

**Original 6×6 Matrix with 4 Processes (Q=2):**

```
┌─────────┬─────────┐
│ A₀₀ A₀₁ │ A₀₂ A₀₃ │  Process 0: A₀₀-A₀₁  Process 1: A₀₂-A₀₃
│ A₁₀ A₁₁ │ A₁₂ A₁₃ │            A₁₀-A₁₁            A₁₂-A₁₃
├─────────┼─────────┤
│ A₂₀ A₂₁ │ A₂₂ A₂₃ │  Process 2: A₂₀-A₂₁  Process 3: A₂₂-A₂₃
│ A₃₀ A₃₁ │ A₃₂ A₃₃ │            A₃₀-A₃₁            A₃₂-A₃₃
└─────────┴─────────┘
```

**Each process gets:** 3×3 submatrix as flat array
**Process 0:** `[A₀₀, A₀₁, A₀₂, A₁₀, A₁₁, A₁₂, A₂₀, A₂₁, A₂₂]`

---

## Slide 7: Phase 3 - MPI Topology Setup

### Step 3.1: Create 2D Process Grid (`fox.c:57-97`)
```c
int setup_grid(struct FoxMPI* fox_mpi) {
    // 1. Create custom datatype for submatrix
    int perProcessMatrixSize = fox_details->N / fox_details->Q;
    MPI_Type_contiguous(perProcessMatrixSize * perProcessMatrixSize, 
                       MATRIX_ELEMENT_MPI, &fox_mpi->datatype);
    
    // 2. Create 2D Cartesian grid
    int dimensions[2] = {Q, Q};
    int periods[2] = {1, 1};  // Circular in both dimensions
    MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, periods, 1, &fox_mpi->cart);
    
    // 3. Get coordinates in grid
    MPI_Cart_coords(fox_mpi->cart, rank, 2, coordinates);
    fox_mpi->fox_details.myRow = coordinates[0];
    fox_mpi->fox_details.myColumn = coordinates[1];
}
```

### Step 3.2: Create Row/Column Communicators
```c
// Row communicator: vary column, fix row
int row_coords[2] = {0, 1};
MPI_Cart_sub(fox_mpi->cart, row_coords, &fox_mpi->row);

// Column communicator: vary row, fix column  
int col_coords[2] = {1, 0};
MPI_Cart_sub(fox_mpi->cart, col_coords, &fox_mpi->col);
```

---

## Slide 8: MPI Topology Transformation

**Before (Linear):** Process ranks 0, 1, 2, 3, 4, 5, 6, 7, 8...

**After (2D Grid for 9 processes):**
```
     Col 0  Col 1  Col 2
Row 0:  0  ←→  1  ←→  2     ← Row Communicator 0
        ↕      ↕      ↕
Row 1:  3  ←→  4  ←→  5     ← Row Communicator 1  
        ↕      ↕      ↕
Row 2:  6  ←→  7  ←→  8     ← Row Communicator 2
        ↑      ↑      ↑
    Col Comm Col Comm Col Comm
        0      1      2
```

**Benefits:**
- **Row broadcasts:** Efficient within-row communication
- **Column shifts:** Efficient within-column communication
- **Circular topology:** No edge processes

---

## Slide 9: Phase 4 - Data Distribution

### Step 4.1: Scatter Operation (`main.c:107`)
```c
MPI_Scatter(dividedMatrix,     // Source (root has all submatrices)
           1,                  // Send 1 unit
           fox_mpi->datatype,  // Custom datatype (submatrix)
           localA,             // Destination buffer
           1,                  // Receive 1 unit
           fox_mpi->datatype,  // Same datatype
           ROOT,               // Sender
           fox_mpi->cart);     // 2D grid communicator
```

### Step 4.2: Initial Setup
```c
copy_matrix(per_process_size, localB, localA);      // B ← A (initial)
copy_matrix(per_process_size, localC, localA);      // C ← A (distances)
```

**Result:** Each process has:
- **localA:** Its submatrix for multiplication
- **localB:** Copy for shifting operations  
- **localC:** Result matrix (initialized with distances)

---

## Slide 10: Phase 5 - Fox Algorithm Core

### All-Pairs Shortest Path Wrapper (`fox.c:216-233`)
```c
void performAllPairsShortestPath(struct FoxMPI* fox_mpi, Matrix localA, Matrix localB, Matrix localC) {
    int m = 1;
    copy_matrix(per_process_size, localC, localA);  // Initialize C with A
    
    while (m < fox_mpi->fox_details.N - 1) {
        performFoxAlgorithm(fox_mpi, localA, localB, localC);  // One iteration
        
        // Update matrices for next iteration
        copy_matrix(per_process_size, localA, localC);
        copy_matrix(per_process_size, localB, localC);
        
        m *= 2;  // Double path length each iteration
    }
}
```

**Key Insight:** Repeated squaring - each iteration considers paths of length 2^k

---

## Slide 11: Fox Algorithm Detail - Single Iteration

### One Fox Iteration (`fox.c:167-214`)
```c
void performFoxAlgorithm(struct FoxMPI* fox_mpi, Matrix localA, Matrix localB, Matrix localC) {
    for (int step = 0; step < fox_mpi->fox_details.Q; step++) {
        // 1. BROADCAST PHASE
        int bcast_root = (fox_mpi->fox_details.myRow + step) % fox_mpi->fox_details.Q;
        
        if (bcast_root == fox_mpi->fox_details.myColumn) {
            // I am broadcaster this step
            MPI_Bcast(localA, 1, fox_mpi->datatype, bcast_root, fox_mpi->row);
            multiply_matrix(gd, localA, localB, localC);  // Use my localA
        } else {
            // Receive broadcast from another process
            MPI_Bcast(tempA, 1, fox_mpi->datatype, bcast_root, fox_mpi->row);
            multiply_matrix(gd, tempA, localB, localC);   // Use received tempA
        }
        
        // 2. SHIFT PHASE
        int source = (myRow + 1) % Q;
        int dest = (myRow - 1 + Q) % Q;
        MPI_Sendrecv_replace(localB, 1, fox_mpi->datatype, dest, 37, source, 37, 
                            fox_mpi->col, &status);
    }
}
```

---

## Slide 12: Fox Algorithm Visual - Step by Step

**Initial State (Q=2, 4 processes):**
```
Process Grid:    A Matrices:     B Matrices:
┌─────┬─────┐    ┌─────┬─────┐   ┌─────┬─────┐
│ P₀  │ P₁  │    │ A₀₀ │ A₀₁ │   │ A₀₀ │ A₀₁ │
├─────┼─────┤    ├─────┼─────┤   ├─────┼─────┤
│ P₂  │ P₃  │    │ A₁₀ │ A₁₁ │   │ A₁₀ │ A₁₁ │
└─────┴─────┘    └─────┴─────┘   └─────┴─────┘
```

**Step 0:** Broadcast root = (row + 0) % 2
- Row 0: P₀ broadcasts A₀₀ to P₁
- Row 1: P₂ broadcasts A₁₀ to P₃
- Multiply: P₀: A₀₀×A₀₀, P₁: A₀₀×A₀₁, P₂: A₁₀×A₁₀, P₃: A₁₀×A₁₁
- Shift B upward in columns

**Step 1:** Broadcast root = (row + 1) % 2  
- Row 0: P₁ broadcasts A₀₁ to P₀
- Row 1: P₃ broadcasts A₁₁ to P₂
- Continue...

---

## Slide 13: Special Matrix Multiplication for Shortest Paths

### Standard Matrix Multiplication:
```c
C[i][j] += A[i][k] * B[k][j]  // Sum of products
```

### Our Shortest Path Multiplication (`matrix.c:109-127`):
```c
MatrixElement min(MatrixElement a, MatrixElement b) {
    return a < b ? a : b;
}

// Min-plus algebra: C[i][j] = min(C[i][j], A[i][k] + B[k][j])
for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
        for (int k = 0; k < matrixSize; k++) {
            MatrixElement current = final_matrix[calculateProjection(matrixSize, i, j)];
            MatrixElement new_path = matrix_1[calculateProjection(matrixSize, i, k)] + 
                                   matrix_2[calculateProjection(matrixSize, k, j)];
            final_matrix[calculateProjection(matrixSize, i, j)] = min(current, new_path);
        }
    }
}
```

**Key:** Replace `+` with `min` and `×` with `+` for shortest path semantics

---

## Slide 14: Phase 6 - Result Collection

### Step 6.1: Gather Results (`main.c:116`)
```c
MPI_Gather(localC,              // Source: each process sends result
          1,                    // Send 1 submatrix
          fox_mpi->datatype,    // Submatrix datatype
          dividedMatrix,        // Destination: root collects all
          1,                    // Receive 1 from each
          fox_mpi->datatype,    // Same datatype
          ROOT,                 // Collector (root process)
          fox_mpi->cart);       // 2D grid communicator
```

### Step 6.2: Matrix Assembly (`fox.c:235-256`)
```c
int assembleMatrix(int matrixSize, int Q, int processes, Matrix originalMatrix, Matrix destinationMatrix) {
    int k = 0;
    for (int proc = 0; proc < processes; proc++) {
        // Calculate where this submatrix belongs in full matrix
        int startRow = (proc / Q) * perMatrix;
        int startCol = (proc % Q) * perMatrix;
        
        // Copy submatrix back to correct position
        for (int i = 0; i < perMatrix; i++) {
            for (int j = 0; j < perMatrix; j++) {
                int globalRow = startRow + i;
                int globalCol = startCol + j;
                destinationMatrix[globalRow * matrixSize + globalCol] = originalMatrix[k++];
            }
        }
    }
}
```

---

## Slide 15: Performance Analysis and Complexity

### Time Complexity Breakdown:
- **Sequential Floyd-Warshall:** O(N³)
- **Our parallel approach:** O(N³/P + T_comm)

### Communication Analysis:
- **Broadcasts per iteration:** Q steps × Q processes = O(Q²)
- **Shifts per iteration:** Q steps × constant = O(Q)
- **Total iterations:** O(log N) for repeated squaring
- **Overall communication:** O(Q² × log N)

### Memory Distribution:
- **Total memory:** N² elements
- **Per process:** (N/Q)² = N²/P elements
- **Perfect load balancing**

### Scalability Constraints:
- **Process count:** Must be perfect square
- **Matrix size:** Must be divisible by √P
- **Network topology:** Benefits from 2D mesh/torus

---

## Slide 16: Practical Considerations and Optimization

### Error Handling in Implementation:
```c
// Validation before execution
if (possibleProCount != envData->processors) {
    fprintf(stderr, "Error: Number of processes (%d) is not a perfect square\n", 
            envData->processors);
    MPI_Abort(MPI_COMM_WORLD, 1);
}

if (matrixSize % maxQ != 0) {
    fprintf(stderr, "Error: Matrix size (%d) is not divisible by sqrt(processes) (%d)\n", 
            matrixSize, maxQ);
    MPI_Abort(MPI_COMM_WORLD, 1);
}
```

### Memory Management:
```c
// Proper cleanup sequence
free_matrix(&dividedMatrix);
free_matrix(&localA);
free_matrix(&localB); 
free_matrix(&localC);
free(fox_details);
free_fox_mpi(fox_mpi);  // Includes MPI object cleanup
MPI_Finalize();
```

### Usage Example:
```bash
# Compile
mpicc -o fox_algorithm main.c fox.c matrix.c

# Run with 9 processes for 12×12 matrix
mpirun -np 9 ./fox_algorithm < input_matrix.txt
```

---

## Slide 17: Summary and Key Takeaways

### What We Accomplished:
1. **Parallelized shortest path computation** using Fox's matrix multiplication
2. **Implemented efficient MPI communication patterns** (broadcast, shift, scatter/gather)
3. **Achieved O(N²/P) memory distribution** with perfect load balancing
4. **Used repeated squaring** to reduce iteration count from O(N) to O(log N)

### Key MPI Concepts Demonstrated:
- **2D Cartesian topologies** for structured communication
- **Custom datatypes** for efficient matrix communication
- **Collective operations** (broadcast, scatter, gather)
- **Point-to-point communication** (sendrecv for shifts)

### Real-World Applications:
- **Network routing protocols** (shortest path in networks)
- **Transportation systems** (optimal route planning)
- **Game pathfinding** (AI movement in games)
- **Social network analysis** (shortest connection paths)

### Performance Benefits:
- **Scalable to large matrices** with sufficient processes
- **Efficient communication patterns** minimize overhead
- **Load-balanced computation** across all processes
- **Logarithmic iteration count** vs linear in sequential

---