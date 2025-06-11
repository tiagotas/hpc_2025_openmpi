
## Instalar depêndencias:
- `sudo apt update && sudo apt upgrade -y`
- `sudo apt install build-essential -y`
- `sudo apt install openmpi-bin libopenmpi-dev -y`

## Testar Dependencias

`mpicc --version`
`mpirun --version`

## Para compilar:

`Compilar: mpicc src/hello_mpi.c -o bin/hello_mpi`


## Para executar: 

`mpirun: mpirun -np 4 bin/hello_mpi`