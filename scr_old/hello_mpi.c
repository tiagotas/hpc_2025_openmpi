#include <mpi.h>
#include <stdio.h>

// Compilar: mpicc hello_mpi.c -o hello_mpi
// Execute com mpirun: mpirun -np 4 ./hello_mpi
// Saída esperada:	
// Hello from process 0 of 4
// Hello from process 1 of 4
// ...

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    printf("Hello from process %d of %d\n", world_rank, world_size);

    MPI_Finalize();
    return 0;
}
