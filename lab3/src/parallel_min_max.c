#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // Добавлена проверка аргумента seed
            if (seed <= 0) {
              printf("seed must be a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            // Добавлена проверка аргумента array_size 
            if (array_size <= 0) {
              printf("array_size must be a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            // Добавлена проверка аргумента pnum 
            if (pnum <= 0) {
              printf("pnum must be a positive number\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  // ДЛЯ ЗАДАНИЯ 3: Выделение памяти под массив пайпов 
  int (*pipes)[2] = malloc(sizeof(int[2]) * pnum);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    // ДЛЯ ЗАДАНИЯ 3: Создание пайпа, если не используются файлы 
    if (!with_files) {
      if (pipe(pipes[i]) < 0) {
        printf("Pipe failed!\n");
        free(array);
        free(pipes);
        return 1;
      }
    }

    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        
        // Вычисление диапазона индекса для каждого процесса 
        unsigned int begin = (array_size / pnum) * i;
        unsigned int end = (i == pnum - 1) ? array_size : (array_size / pnum) * (i + 1);

        // Поиск минимума и максимума в своем диапазоне
        struct MinMax local_result = GetMinMax(array, begin, end);

        if (with_files) {
          // Запись результатов работы дочернего процесса в файл 
          char file_name[50];
          sprintf(file_name, "min_max_%d.txt", i);
          FILE *fp = fopen(file_name, "w");
          if (fp == NULL) {
            printf("Error opening file for writing!\n");
            free(array);
            if (!with_files) free(pipes);
            return 1;
          }
          fprintf(fp, "%d %d", local_result.min, local_result.max);
          fclose(fp);
        } else {
          //  ДЛЯ ЗАДАНИЯ 3: Запись результатов в канал 
          close(pipes[i][0]); // Закрываем ненужный конец чтения в дочернем процессе
          write(pipes[i][1], &local_result.min, sizeof(local_result.min));
          write(pipes[i][1], &local_result.max, sizeof(local_result.max));
          close(pipes[i][1]);
        }

        free(array);
        if (!with_files) free(pipes);
        return 0;
      } else {
        // ДЛЯ ЗАДАНИЯ 3: Родитель закрывает ненужный конец записи 
        if (!with_files) {
          close(pipes[i][1]);
        }
      }

    } else {
      printf("Fork failed!\n");
      free(array);
      free(pipes);
      return 1;
    }
  }

  // Ожидание завершения всех дочерних процессов 
  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // Чтение результатов дочерних процессов из файлов в родителе 
      char file_name[50];
      sprintf(file_name, "min_max_%d.txt", i);
      FILE *fp = fopen(file_name, "r");
      if (fp != NULL) {
        fscanf(fp, "%d %d", &min, &max);
        fclose(fp);
      }
    } else {
      // ДЛЯ ЗАДАНИЯ 3: Чтение результатов из канала 
      read(pipes[i][0], &min, sizeof(min));
      read(pipes[i][0], &max, sizeof(max));
      close(pipes[i][0]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  //  ДЛЯ ЗАДАНИЯ 3: Освобождение памяти массива пайпов 
  if (!with_files) {
    free(pipes);
  }

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}