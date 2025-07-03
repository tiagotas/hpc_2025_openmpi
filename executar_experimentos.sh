#!/bin/bash

BIN=bin/paralelo
ENTRADAS=entradas
SAIDAS=saidas
LOGS=logs

mkdir -p $SAIDAS
mkdir -p $LOGS

# Tamanhos de matriz e número de processos
tamanhos=(256 512 768)
threads=(2 4 8 16)

# Limpa logs antigos
rm -f logs/*.log

# Função para calcular média
media() {
    awk '{s+=$1} END {print s/NR}'
}

# Teste SEQUENCIAL
for size in "${tamanhos[@]}"; do
    echo "Testando versão sequencial com matriz ${size}x${size}"
    logfile="logs/seq_${size}.log"
    for i in {1..10}; do
        ./bin/sequencial entradas/entrada_${size}.txt saidas/saida_seq_${size}.txt | grep "Tempo" | awk '{print $2}' >> "$logfile"
    done
    echo -n "Média sequencial (${size}x${size}): "
    media < "$logfile"
done

for size in "${tamanhos[@]}"; do
  entrada="${ENTRADAS}/entrada_${size}.txt"
  
  for t in "${threads[@]}"; do
    log_file="${LOGS}/par_${size}_${t}.log"
    echo "Executando: matriz ${size}x${size}, ${t} processos"
    
    > "$log_file"  # limpa o log anterior

    for i in $(seq 1 10); do
      #echo -n "Execução $i: " >> "$log_file"
      echo -n "" >> "$log_file"
      mpirun --oversubscribe -np $t $BIN "$entrada" "${SAIDAS}/saida_${size}_${t}.txt" | grep Tempo | awk '{print $2}' >> "$log_file"
    done
    echo -n "Média paralela (${size}x${size}, ${t} processos): "
    media < "$logfile"
  done
done
