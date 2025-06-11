#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// gcc stencil_seq.c -o stencil_seq -O2

// mpicc sequencial.c -o bin/sequencial -fopenmp -O2
//    mpicc: compila com suporte a MPI.
//    -fopenmp: ativa o suporte ao OpenMP dentro de cada processo MPI.
//    -O2: ativa otimizações.

// Executar: mpirun -np 4 ./stencil_mpi_omp entrada.txt saida_paralela.txt

#define ITER 1000
#define BORDER_COLOR 127

typedef struct {
    unsigned char R, G, B;
    int fixed;
} Pixel;

void alocaImagem(Pixel ***img, int N) {
    *img = malloc(N * sizeof(Pixel *));
    for (int i = 0; i < N; i++) {
        (*img)[i] = malloc(N * sizeof(Pixel));
        for (int j = 0; j < N; j++) {
            (*img)[i][j] = (Pixel){0, 0, 0, 0};
        }
    }
}

void liberaImagem(Pixel **img, int N) {
    for (int i = 0; i < N; i++) free(img[i]);
    free(img);
}

void aplicaBorda(Pixel **img, int N) {
    for (int i = 0; i < N; i++) {
        img[0][i].R = img[0][i].G = img[0][i].B = BORDER_COLOR;
        img[N-1][i].R = img[N-1][i].G = img[N-1][i].B = BORDER_COLOR;
        img[i][0].R = img[i][0].G = img[i][0].B = BORDER_COLOR;
        img[i][N-1].R = img[i][N-1].G = img[i][N-1].B = BORDER_COLOR;
    }
}

void copiaImagem(Pixel **dest, Pixel **src, int N) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            dest[i][j] = src[i][j];
}

void estencil(Pixel **curr, Pixel **next, int N) {
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < N-1; j++) {
            if (curr[i][j].fixed) continue;
            next[i][j].R = (curr[i][j].R + curr[i-1][j].R + curr[i+1][j].R + curr[i][j-1].R + curr[i][j+1].R) / 5;
            next[i][j].G = (curr[i][j].G + curr[i-1][j].G + curr[i+1][j].G + curr[i][j-1].G + curr[i][j+1].G) / 5;
            next[i][j].B = (curr[i][j].B + curr[i-1][j].B + curr[i+1][j].B + curr[i][j-1].B + curr[i][j+1].B) / 5;
        }
    }
}

void salvaSaida(Pixel **img, int N, const char *filename) {
    FILE *f = fopen(filename, "w");
    for (int i = 0; i < N; i++) {
        for (int j = 64; j < 128; j++) {
            fprintf(f, "%d %d %d ", img[i][j].R, img[i][j].G, img[i][j].B);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Uso: %s <entrada.txt> <saida.txt>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    int N, numFixos;
    fscanf(f, "%d %d", &N, &numFixos);

    Pixel **img1, **img2;
    alocaImagem(&img1, N);
    alocaImagem(&img2, N);

    aplicaBorda(img1, N);
    aplicaBorda(img2, N);

    for (int i = 0; i < numFixos; i++) {
        int x, y, r, g, b;
        fscanf(f, "%d %d %d %d %d", &x, &y, &r, &g, &b);
        img1[x][y] = (Pixel){r, g, b, 1};
        img2[x][y] = img1[x][y];
    }
    fclose(f);

    clock_t inicio = clock();
    for (int k = 0; k < ITER; k++) {
        estencil(img1, img2, N);
        Pixel **tmp = img1;
        img1 = img2;
        img2 = tmp;
    }
    clock_t fim = clock();

    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo: %.4f segundos\n", tempo);

    salvaSaida(img1, N, argv[2]);
    liberaImagem(img1, N);
    liberaImagem(img2, N);
    return 0;
}
