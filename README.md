
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

## Resultados Intel Core i5 7200U

| Tamanho da Matriz | Sequencial (s) | Paralelo 2P | Paralelo 4P | Paralelo 8P | Paralelo 16P |
|-------------------|----------------|-------------|-------------|-------------|---------------|
| 256x256 | 2.12223 | 1.11456 | 3.75311 | 3.07589 | 9.61958 |
| 512x512 | 6.98428 | 4.17279 | 5.63497 | 6.49305 | 7.63082 |
| 768x768 | 15.71370 | 9.36150 | 13.87079 | 10.42600 | 23.23195 |

## Resultados Amd Ryzen 7 5800H

| Tamanho da Matriz | Sequencial (s) | Paralelo 2P | Paralelo 4P | Paralelo 8P | Paralelo 16P |
|-------------------|----------------|-------------|-------------|-------------|---------------|
| 256x256 | 2.34576 | 0.49997 | 0.25218 | 0.20611 | 0.56618 |
| 512x512 | 5.08784 | 1.65534 | 1.07375 | 0.81502 | 0.50957 |
| 768x768 | 11.17065 | 3.63306 | 2.51430 | 1.81163 | 1.81153 |