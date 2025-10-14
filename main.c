#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mpi/mpi.h>
#include <math.h>

#define INF 1000000000LL
#define MAXN 1200


int main(int argc, char* argv[]) {
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int N;
  long long* fullMatrix = NULL;  // usada só no rank 0
  long long* localBlock = NULL;

  if (rank == 0) {
    // === STEP 1: Ler matriz de entrada ===
    if (scanf("%d", &N) != 1) {
      fprintf(stderr, "Erro ao ler N.\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fullMatrix = malloc(N * N * sizeof(long long));
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        int val;
        scanf("%d", &val);
        if (i == j)
          fullMatrix[ i * N + j ] = 0;
        else if (val == 0)
          fullMatrix[ i * N + j ] = INF;
        else
          fullMatrix[ i * N + j ] = val;
      }
    }
  }

  // === STEP 2: Distribuir dados ===

  // 1. Broadcast de N para todos
  MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // 2. Verificar se o número de processos é quadrado perfeito
  int Q = (int)(sqrt(size) + 0.5);
  if (Q * Q != size) {
    if (rank == 0)
      fprintf(stderr, "Erro: número de processos (%d) não é quadrado perfeito.\n", size);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // 3. Verificar se N é divisível por Q
  if (N % Q != 0) {
    if (rank == 0)
      fprintf(stderr, "Erro: N=%d não é divisível por Q=%d.\n", N, Q);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // 4. Calcular tamanho do bloco local
  int nb = N / Q;

  // Alocar memória local
  localBlock = malloc(nb * nb * sizeof(long long));

  // 5. Scatter manual (envio de blocos)
  if (rank == 0) {
    for (int bi = 0; bi < Q; bi++) {
      for (int bj = 0; bj < Q; bj++) {
        int dest = bi * Q + bj;
        for (int i = 0; i < nb; i++) {
          for (int j = 0; j < nb; j++) {
            int global_i = bi * nb + i;
            int global_j = bj * nb + j;
            long long value = fullMatrix[ global_i * N + global_j ];
            if (dest == 0)
              localBlock[ i * nb + j ] = value;
            else
              MPI_Send(&value, 1, MPI_LONG_LONG, dest, 0, MPI_COMM_WORLD);
          }
        }
      }
    }
  }
  else {
    for (int i = 0; i < nb * nb; i++) {
      MPI_Recv(&localBlock[ i ], 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }

  // Confirmar que cada processo recebeu o seu bloco (debug)
  printf("Rank %d recebeu bloco %dx%d.\n", rank, nb, nb);

  // libertar memória root
  if (rank == 0)
    free(fullMatrix);

  free(localBlock);
  MPI_Finalize();
  return 0;
}