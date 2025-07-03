#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_N 1024
#define ITERATIONS 1000

/*
Passo 1: Compilar o código
  gcc -o bin/sequencial src/sequencial.c -lm

Passo 2: Executar o código
  ./bin/sequencial entradas/entrada_256.txt saidas/saida_seq_256.txt

*/

typedef struct {
    unsigned char r, g, b;
    int is_fixed;
} Pixel;

Pixel A[MAX_N][MAX_N], B[MAX_N][MAX_N];
int N, num_fixed;

void read_input(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Erro ao abrir arquivo de entrada");
        exit(1);
    }

    fscanf(f, "%d %d", &N, &num_fixed);

    // Inicializa a matriz com preto (0,0,0)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = (Pixel){0, 0, 0, 0};

    // Define a borda com valores fixos (127,127,127)
    for (int i = 0; i < N; i++) {
        A[i][0] = A[i][N - 1] = (Pixel){127, 127, 127, 1};
        A[0][i] = A[N - 1][i] = (Pixel){127, 127, 127, 1};
    }

    // Lê os pontos fixos do arquivo
    for (int i = 0; i < num_fixed; i++) {
        int x, y, r, g, b;
        fscanf(f, "%d %d %d %d %d", &x, &y, &r, &g, &b);
        A[x][y] = (Pixel){r, g, b, 1};
    }

    fclose(f);
}

void write_output(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Erro ao abrir arquivo de saída");
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 64; j < 128; j++) {
            fprintf(f, "%d %d %d ", A[i][j].r, A[i][j].g, A[i][j].b);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    read_input(argv[1]);

    clock_t start = clock();

    for (int it = 0; it < ITERATIONS; it++) {
        for (int i = 1; i < N - 1; i++) {
            for (int j = 1; j < N - 1; j++) {
                if (A[i][j].is_fixed) {
                    B[i][j] = A[i][j];
                    continue;
                }

                int sum_r = A[i][j].r + A[i - 1][j].r + A[i + 1][j].r + A[i][j - 1].r + A[i][j + 1].r;
                int sum_g = A[i][j].g + A[i - 1][j].g + A[i + 1][j].g + A[i][j - 1].g + A[i][j + 1].g;
                int sum_b = A[i][j].b + A[i - 1][j].b + A[i + 1][j].b + A[i][j - 1].b + A[i][j + 1].b;

                B[i][j].r = sum_r / 5;
                B[i][j].g = sum_g / 5;
                B[i][j].b = sum_b / 5;
                B[i][j].is_fixed = 0;
            }
        }

        // Copia B de volta para A
        for (int i = 1; i < N - 1; i++)
            for (int j = 1; j < N - 1; j++)
                if (!A[i][j].is_fixed)
                    A[i][j] = B[i][j];
    }

    clock_t end = clock();

    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Tempo: %.5f segundos\n", elapsed);

    write_output(argv[2]);
    return 0;
}