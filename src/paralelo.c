#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_N 1024
#define ITERATIONS 1000

/*
Passo 1: Incluir as bibliotecas necessárias
 sudo apt update
 sudo apt install -y openmpi-bin libopenmpi-dev
 mpicc --version

Passo 2: Compilar o código
 mpicc -o bin/paralelo src/paralelo.c -lm

Passo 3: Executar o código
 --oversubscribe para executar no WSL
 mpirun --oversubscribe -np <num_processos> bin/paralelo entradas/entrada_256.txt saidas/saida_256.txt
 mpirun --oversubscribe -np 4 bin/paralelo entradas/entrada_256.txt saidas/saida_256.txt
*/

typedef struct {
    unsigned char r, g, b;
    int is_fixed;
} Pixel;

Pixel A[MAX_N][MAX_N]; // Usado apenas no rank 0
int N, num_fixed;

void read_input(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Erro ao abrir arquivo de entrada");
        exit(1);
    }

    fscanf(f, "%d %d", &N, &num_fixed);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = (Pixel){0, 0, 0, 0};

    for (int i = 0; i < N; i++) {
        A[i][0] = A[i][N - 1] = (Pixel){127, 127, 127, 1};
        A[0][i] = A[N - 1][i] = (Pixel){127, 127, 127, 1};
    }

    for (int i = 0; i < num_fixed; i++) {
        int x, y, r, g, b;
        fscanf(f, "%d %d %d %d %d", &x, &y, &r, &g, &b);
        A[x][y] = (Pixel){r, g, b, 1};
    }

    fclose(f);
}

void write_output(const char* filename, Pixel* full_matrix) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Erro ao abrir arquivo de saída");
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 64; j < 128; j++) {
            Pixel p = full_matrix[i * N + j];
            fprintf(f, "%d %d %d ", p.r, p.g, p.b);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

void exchange_borders(Pixel* local_matrix, int local_rows, int rank, int size) {
    int row_size = N * sizeof(Pixel);
    Pixel* top = local_matrix;
    Pixel* bottom = local_matrix + (local_rows + 1) * N;

    if (rank > 0) {
        MPI_Sendrecv(local_matrix + N, row_size, MPI_BYTE, rank - 1, 0,
                     top, row_size, MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    if (rank < size - 1) {
        MPI_Sendrecv(local_matrix + local_rows * N, row_size, MPI_BYTE, rank + 1, 0,
                     bottom, row_size, MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3) {
        if (rank == 0)
            fprintf(stderr, "Uso: %s <entrada.txt> <saida.txt>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) read_input(argv[1]);

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Cálculo do particionamento
    int *sendcounts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int base = N / size, resto = N % size;
    int offset = 0;

    for (int i = 0; i < size; i++) {
        int linhas = base + (i < resto ? 1 : 0);
        sendcounts[i] = linhas * N * sizeof(Pixel);
        displs[i] = offset * N * sizeof(Pixel);
        offset += linhas;
    }

    int local_rows = sendcounts[rank] / (N * sizeof(Pixel));

    // Matriz local com 2 linhas extras para ghost rows
    Pixel* local_A = calloc((local_rows + 2) * N, sizeof(Pixel));
    Pixel* local_B = calloc((local_rows + 2) * N, sizeof(Pixel));

    // Scatter dos dados
    MPI_Scatterv(rank == 0 ? &A[0][0] : NULL, sendcounts, displs, MPI_BYTE,
                 local_A + N, local_rows * N * sizeof(Pixel), MPI_BYTE,
                 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for (int it = 0; it < ITERATIONS; it++) {
        exchange_borders(local_A, local_rows, rank, size);

        for (int i = 1; i <= local_rows; i++) {
            for (int j = 1; j < N - 1; j++) {
                Pixel p = local_A[i * N + j];
                if (p.is_fixed) {
                    local_B[i * N + j] = p;
                    continue;
                }

                int sum_r = p.r + local_A[(i - 1) * N + j].r + local_A[(i + 1) * N + j].r + local_A[i * N + (j - 1)].r + local_A[i * N + (j + 1)].r;
                int sum_g = p.g + local_A[(i - 1) * N + j].g + local_A[(i + 1) * N + j].g + local_A[i * N + (j - 1)].g + local_A[i * N + (j + 1)].g;
                int sum_b = p.b + local_A[(i - 1) * N + j].b + local_A[(i + 1) * N + j].b + local_A[i * N + (j - 1)].b + local_A[i * N + (j + 1)].b;

                local_B[i * N + j] = (Pixel){sum_r / 5, sum_g / 5, sum_b / 5, 0};
            }
        }

        // Troca ponteiros
        Pixel* temp = local_A;
        local_A = local_B;
        local_B = temp;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    if (rank == 0) {
        Pixel* full_result = malloc(N * N * sizeof(Pixel));
        memcpy(full_result, local_A + N, sendcounts[0]);
        for (int i = 1; i < size; i++) {
            MPI_Recv((char*)full_result + displs[i], sendcounts[i], MPI_BYTE, i, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("Tempo: %.5f segundos\n", end - start);
        write_output(argv[2], full_result);
        free(full_result);
    } else {
        MPI_Send(local_A + N, local_rows * N * sizeof(Pixel), MPI_BYTE, 0, 99, MPI_COMM_WORLD);
    }

    free(local_A);
    free(local_B);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}