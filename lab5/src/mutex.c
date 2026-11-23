#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void do_one_thing(int *);
void do_another_thing(int *);
void do_wrap_up(int);

int common = 0; // Общая переменная (разделяемая память)
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // Мьютекс для взаимного исключения

int main() {
  pthread_t thread1, thread2;

  // Создаем 2 параллельных потока
  if (pthread_create(&thread1, NULL, (void *)do_one_thing, (void *)&common) != 0) {
    perror("pthread_create");
    exit(1);
  }
  if (pthread_create(&thread2, NULL, (void *)do_another_thing, (void *)&common) != 0) {
    perror("pthread_create");
    exit(1);
  }

  // Ожидаем завершения обоих потоков
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  do_wrap_up(common);
  return 0;
}

void do_one_thing(int *pnum_times) {
  int i, work;
  unsigned long k;
  for (i = 0; i < 50; i++) {
#ifndef WITHOUT_MUTEX
    pthread_mutex_lock(&mut); // НАЧАЛО критической секции (захват замка)
#endif
    printf("doing one thing\n");
    work = *pnum_times;
    printf("counter = %d\n", work);
    work++;
    for (k = 0; k < 500000; k++); // Задержка: без мьютекса тут происходит гонка (Race condition)
    *pnum_times = work;
#ifndef WITHOUT_MUTEX
    pthread_mutex_unlock(&mut); // КОНЕЦ критической секции (освобождение замка)
#endif
  }
}

void do_another_thing(int *pnum_times) {
  int i, work;
  unsigned long k;
  for (i = 0; i < 50; i++) {
#ifndef WITHOUT_MUTEX
    pthread_mutex_lock(&mut); // Вход в критическую секцию
#endif
    printf("doing another thing\n");
    work = *pnum_times;
    printf("counter = %d\n", work);
    work++;
    for (k = 0; k < 500000; k++);
    *pnum_times = work;
#ifndef WITHOUT_MUTEX
    pthread_mutex_unlock(&mut); // Выход из критической секции
#endif
  }
}

void do_wrap_up(int counter) {
  printf("All done, counter = %d\n", counter);
}
