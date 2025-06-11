#!/bin/bash

gerar_entrada() {
    tamanho=$1
    num_pontos=$2
    arquivo="entradas/entrada_${tamanho}.txt"

    echo "${tamanho} ${num_pontos}" > "$arquivo"

    for ((i = 0; i < num_pontos; i++)); do
        # Evita bordas fixas (opcional)
        x=$((RANDOM % (tamanho - 2) + 1))
        y=$((RANDOM % (tamanho - 2) + 1))

        r=$((RANDOM % 256))
        g=$((RANDOM % 256))
        b=$((RANDOM % 256))

        echo "$x $y $r $g $b" >> "$arquivo"
    done

    echo "Arquivo $arquivo gerado com $num_pontos pontos fixos."
}

# Exemplo de chamadas
gerar_entrada 256 100
gerar_entrada 512 250
gerar_entrada 768 500