#!/bin/bash

# Diretório dos logs
LOG_DIR="logs"

# Tamanhos da matriz e número de threads testados
SIZES=("256" "512" "768")
THREADS=("2" "4" "8" "16")

# Cabeçalho da tabela Markdown
echo "| Tamanho da Matriz | Sequencial (s) | Paralelo 2T | Paralelo 4T | Paralelo 8T | Paralelo 16T |"
echo "|-------------------|----------------|-------------|-------------|-------------|---------------|"

for size in "${SIZES[@]}"; do
  # Nome do arquivo sequencial correspondente
  SEQ_FILE="${LOG_DIR}/seq_${size}.log"
  
  # Extrai o tempo médio do sequencial
  if [[ -f "$SEQ_FILE" ]]; then
    tempo_seq=$(grep -m 1 -Eo '[0-9]+\.[0-9]+' "$SEQ_FILE")
  else
    tempo_seq="N/A"
  fi

  # Inicializa a linha com o tamanho e tempo sequencial
  linha="| ${size}x${size} | ${tempo_seq}"

  # Para cada número de threads, buscar o respectivo arquivo de log
  for t in "${THREADS[@]}"; do
    PAR_FILE="${LOG_DIR}/par_${size}_${t}.log"
    if [[ -f "$PAR_FILE" ]]; then
      tempo_par=$(grep -m 1 -Eo '[0-9]+\.[0-9]+' "$PAR_FILE")
    else
      tempo_par="N/A"
    fi
    linha="${linha} | ${tempo_par}"
  done

  linha="${linha} |"
  echo "$linha"
done
# Fim do script