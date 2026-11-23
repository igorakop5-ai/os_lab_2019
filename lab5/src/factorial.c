#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Диапазон чисел [start..end] для конкретного потока
struct FactorialArgs {
  uint64_t start;
  uint64_t end;
  uint64_t mod;
};

uint64_t total_result = 1; // Общий накопитель факториала
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // Защита записи в total_result

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  uint64_t partial_prod = 1;

  // 1. Локальное перемножение диапазона (без блокировок)
  for (uint64_t i = fargs->start; i <= fargs->end; i++) {
    partial_prod = (partial_prod * (i % fargs->mod)) % fargs->mod;
  }

  // 2. Критическая секция: объединение результата под мьютексом
  pthread_mutex_lock(&mut);
  total_result = (total_result * partial_prod) % fargs->mod;
  pthread_mutex_unlock(&mut);

  return NULL;
}

int main(int argc, char **argv) {
  int k = -1, pnum = -1, mod = -1;

  // Парсинг параметров: -k, --pnum, --mod
  while (true) {
    static struct option options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "k:p:m:", options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 'k': k = atoi(optarg); break;
      case 'p': pnum = atoi(optarg); break;
      case 'm': mod = atoi(optarg); break;
      default: return 1;
    }
  }

  if (k < 0 || pnum <= 0 || mod <= 0) {
    printf("Usage: %s -k <num> --pnum=<num> --mod=<num>\n", argv[0]);
    return 1;
  }

  total_result = 1 % mod;

  if (k > 1) {
    if (pnum > k) pnum = k; // Ограничение: потоков не больше, чем чисел

    pthread_t threads[pnum];
    struct FactorialArgs args[pnum];

    // Равномерное разбиение [1..k] на pnum частей
    for (int i = 0; i < pnum; i++) {
      args[i].start = 1 + (uint64_t)i * k / pnum;
      args[i].end = (i == pnum - 1) ? (uint64_t)k : (uint64_t)(i + 1) * k / pnum;
      args[i].mod = (uint64_t)mod;

      pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i]);
    }

    // Ожидание всех потоков
    for (int i = 0; i < pnum; i++) {
      pthread_join(threads[i], NULL);
    }
  }

  printf("%d! mod %d = %llu\n", k, mod, (unsigned long long)total_result);
  return 0;
}
