#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdatomic.h>

// Переменная: 0 = свободно, 1 = занято
atomic_int futex_var = 0;
int common = 0;

// Обертка над системным вызовом futex
static int futex(int *uaddr, int op, int val) {
    return syscall(SYS_futex, uaddr, op, val, NULL, NULL, 0);
}

void futex_lock(atomic_int *f) {
    int expected = 0;
    // Атомарно пробуем сменить 0 на 1 в пространстве пользователя (Userspace)
    while (!atomic_compare_exchange_strong(f, &expected, 1)) {
        // Если уже занято (была 1) — обращаемся к ядру, чтобы усыпить поток
        futex((int *)f, FUTEX_WAIT, 1);
        expected = 0;
    }
}

void futex_unlock(atomic_int *f) {
    // Сбрасываем в 0
    atomic_store(f, 0);
    // Будим 1 уснувший поток через ядро Linux
    futex((int *)f, FUTEX_WAKE, 1);
}

void *thread_func(void *arg) {
    for (int i = 0; i < 50; i++) {
        futex_lock(&futex_var); // Захват через futex

        int work = common;
        work++;
        for (unsigned long k = 0; k < 500000; k++);
        common = work;

        futex_unlock(&futex_var); // Освобождение через futex
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Result with Futex: counter = %d\n", common);
    return 0;
}
