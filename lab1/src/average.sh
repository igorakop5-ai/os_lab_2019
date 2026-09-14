#!/bin/bash

# Количество аргументов
count=$#

# Если аргументов нет — выходим
if [ $count -eq 0 ]; then
    echo "Нет аргументов"
    exit 1
fi

# Сумма всех аргументов
sum=0
for arg in "$@"; do
    sum=$((sum + arg))
done

# Среднее арифметическое
average=$((sum / count))

echo "Количество: $count"
echo "Среднее арифметическое: $average"