#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Два общих ресурса (мьютекса)
pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;

// Поток 1: берет A, затем ждет B
void *thread_one(void *arg) {
  pthread_mutex_lock(&mutex_a);
  printf("[Thread 1] Захватил Mutex A\n");
  fflush(stdout);

  sleep(1); // Даем второму потоку успеть взять Mutex B

  printf("[Thread 1] Ждет Mutex B...\n");
  fflush(stdout);
  pthread_mutex_lock(&mutex_b); // Зависает: Mutex B занят Потоком 2

  pthread_mutex_unlock(&mutex_b);
  pthread_mutex_unlock(&mutex_a);
  return NULL;
}

// Поток 2: берет B, затем ждет A (обратный порядок!)
void *thread_two(void *arg) {
  pthread_mutex_lock(&mutex_b);
  printf("[Thread 2] Захватил Mutex B\n");
  fflush(stdout);

  sleep(1); // Даем первому потоку успеть взять Mutex A

  printf("[Thread 2] Ждет Mutex A...\n");
  fflush(stdout);
  pthread_mutex_lock(&mutex_a); // Зависает: Mutex A занят Потоком 1

  pthread_mutex_unlock(&mutex_a);
  pthread_mutex_unlock(&mutex_b);
  return NULL;
}

int main() {
  pthread_t t1, t2;

  printf("=== Демонстрация Deadlock ===\n");
  fflush(stdout);

  pthread_create(&t1, NULL, thread_one, NULL);
  pthread_create(&t2, NULL, thread_two, NULL);

  // Круговое ожидание: потоки никогда не завершатся
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  return 0;
}
