#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> /* для waitpid */

int main(void)
{
    pid_t chpid;
    pid_t pid, ppid;
    int a = 0;

    chpid = fork();
    if (chpid < 0) {
        /* ошибка при fork */
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (chpid == 0) {
        /* дочерний процесс */
        a = a + 1;
        pid  = getpid();
        ppid = getppid();

        printf("Child: pid = %d, ppid = %d, a = %d\n", (int)pid, (int)ppid, a);
        /* дочерний может завершиться */
        return 0;
    }
    else {
        /* родительский процесс */
        pid  = getpid();
        ppid = getppid();

        /* можно дождаться завершения дочернего, чтобы не оставить зомби */
        if (waitpid(chpid, NULL, 0) == -1) {
            perror("waitpid");
        }

        printf("Parent: My pid = %d, my ppid = %d, a = %d\n", (int)pid, (int)ppid, a);
    }

    return 0;
}
