#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define ITER 1000
#define BORDER_COLOR 127

typedef struct {
    unsigned char R, G, B;
    int fixed;
} Pixel;

void aplicaBorda(Pixel **img, int rows, int N) {
    for (int i = 0; i < rows; i++) {
        img[i][0].R = img[i][0].G = img[i][0].B = BORDER_COLOR;
        img[i][N-1].R = img[i][N-1].G = img[i][N-1].B = BORDER_COLOR;
    }
}

void stencil(Pixel **curr, Pixel **next, int rows, int N) {
    #pragma omp parallel for
    for (int i = 1; i < rows-1; i++) {
        for (int j = 1; j < N-1; j++) {
            if (curr[i][j].fixed) continue;
            next[i][j].R = (curr[i][j].R + curr[i-1][j].R + curr[i+1][j].R + curr[i][j-1].R + curr[i][j+1].R) / 5;
            next[i][j].G = (curr[i][j].G + curr[i-1][j].G + curr[i+1][j].G + curr[i][j-1].G + curr[i][j+1].G) / 5;
            next[i][j].B = (curr[i][j].B + curr[i-1][j].B + curr[i+1][j].B + curr[i][j-1].B + curr[i][j+1].B) / 5;
        }
    }
}

Pixel **alocaImagem(int rows, int cols) {
    Pixel **img = malloc(rows * sizeof(Pixel *));
    for (int i = 0; i < rows; i++) {
        img[i] = malloc(cols * sizeof(Pixel));
        for (int j = 0; j < cols; j++) {
            img[i][j] = (Pixel){0, 0, 0, 0};
        }
    }
    return img;
}

void liberaImagem(Pixel **img, int rows) {
    for (int i = 0; i < rows; i++) free(img[i]);
    free(img);
}

void salvaSaida(Pixel **img, int N, const char *nome) {
    FILE *f = fopen(nome, "w");
    for (int i = 0; i < N; i++) {
        for (int j = 64; j < 128; j++) {
            fprintf(f, "%d %d %d ", img[i][j].R, img[i][j].G, img[i][j].B);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3) {
        if (rank == 0) printf("Uso: %s <entrada.txt> <saida.txt>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int N, numFixos;
    Pixel **global = NULL;

    if (rank == 0) {
        FILE *f = fopen(argv[1], "r");
        fscanf(f, "%d %d", &N, &numFixos);
        global = alocaImagem(N, N);

        for (int i = 0; i < numFixos; i++) {
            int x, y, r, g, b;
            fscanf(f, "%d %d %d %d %d", &x, &y, &r, &g, &b);
            global[x][y] = (Pixel){r, g, b, 1};
        }
        fclose(f);
        for (int i = 0; i < N; i++) {
            global[i][0].R = global[i][N-1].R = BORDER_COLOR;
            global[i][0].G = global[i][N-1].G = BORDER_COLOR;
            global[i][0].B = global[i][N-1].B = BORDER_COLOR;
        }
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int linhasLocal = N / size;
    int linhasComGhost = linhasLocal + 2;

    Pixel **curr = alocaImagem(linhasComGhost, N);
    Pixel **next = alocaImagem(linhasComGhost, N);

    MPI_Datatype pixel_type;
    MPI_Type_contiguous(4, MPI_UNSIGNED_CHAR); // R, G, B + fixed como char
    MPI_Type_commit(&pixel_type);

    for (int i = 0; i < linhasLocal; i++) {
        if (rank == 0) {
            memcpy(curr[i+1], global[i], sizeof(Pixel) * N);
        }
        MPI_Scatter(&global[i + rank * linhasLocal][0], N * linhasLocal * sizeof(Pixel), MPI_BYTE,
                    &curr[1][0], N * linhasLocal * sizeof(Pixel), MPI_BYTE,
                    0, MPI_COMM_WORLD);
    }

    double inicio = MPI_Wtime();

    for (int it = 0; it < ITER; it++) {
        // Troca de ghost rows
        if (rank > 0) {
            MPI_Send(curr[1], N * sizeof(Pixel), MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD);
            MPI_Recv(curr[0], N * sizeof(Pixel), MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (rank < size - 1) {
            MPI_Send(curr[linhasLocal], N * sizeof(Pixel), MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(curr[linhasLocal + 1], N * sizeof(Pixel), MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        aplicaBorda(curr, linhasComGhost, N);
        stencil(curr, next, linhasComGhost, N);

        // swap
        Pixel **tmp = curr; curr = next; next = tmp;
    }

    double fim = MPI_Wtime();
    double tempo = fim - inicio;

    if (rank == 0) printf("Tempo total: %.4f s\n", tempo);

    // Coleta dos resultados
    if (rank == 0) {
        for (int i = 0; i < linhasLocal; i++)
            memcpy(global[i], curr[i+1], sizeof(Pixel) * N);
        for (int p = 1; p < size; p++) {
            for (int i = 0; i < linhasLocal; i++) {
                MPI_Recv(global[p * linhasLocal + i], N * sizeof(Pixel), MPI_BYTE, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        salvaSaida(global, N, argv[2]);
        liberaImagem(global, N);
    } else {
        for (int i = 1; i <= linhasLocal; i++) {
            MPI_Send(curr[i], N * sizeof(Pixel), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
        }
    }

    liberaImagem(curr, linhasComGhost);
    liberaImagem(next, linhasComGhost);
    MPI_Finalize();
    return 0;
}
