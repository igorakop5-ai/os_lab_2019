#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

int common = 0;
sem_t sem; // Объявляем POSIX-семафор

void *thread_func(void *arg) {
    for (int i = 0; i < 50; i++) {
        sem_wait(&sem); // Захват семафора (декремент счётчика)

        int work = common;
        work++;
        for (unsigned long k = 0; k < 500000; k++); // Задержка
        common = work;

        sem_post(&sem); // Освобождение семафора (инкремент счётчика)
    }
    return NULL;
}

int main() {
    // sem_init(семафор, 0 = разделяется между потоками, 1 = начальное значение)
    sem_init(&sem, 0, 1);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Result with Semaphore: counter = %d\n", common);

    sem_destroy(&sem); // Удаление семафора
    return 0;
}
