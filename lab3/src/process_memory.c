#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс запускает sequential_min_max через execvp
        // Передаем параметры: seed = 42, array_size = 100
        char *args[] = {"./sequential_min_max", "42", "100", NULL};
        execvp(args[0], args);
        
        // Если execvp завершился с ошибкой
        perror("execvp failed");
        return 1;
    } else {
        // Родительский процесс ждет завершения дочернего
        int status;
        waitpid(pid, &status, 0);
        printf("Sequential application successfully launched via fork and exec.\n");
    }

    return 0;
}