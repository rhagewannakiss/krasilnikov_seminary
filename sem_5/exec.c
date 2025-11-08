#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
    pid_t chpid;
    pid_t pid, ppid;
    int a = 0;
    int result;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file-to-cat>\n", argv[0]);
        return 1;
    }

    chpid = fork();
    if (chpid == -1) {
        perror("fork");
        return 1;
    }
    else if (chpid > 0) {
        /* родитель */
        a = a + 1;
        pid  = getpid();
        ppid = getppid();
        printf("Parent: pid = %d, ppid = %d, result = %d\n", (int)pid, (int)ppid, a);
        /* родитель может дождаться ребёнка, но не обязателен в примере */
        return 0;
    }
    else {
        /* дочерний процесс: заменяем образ процесса на /bin/cat */
        /* execle(path, argv0, argv1, ..., NULL, envp) — последний аргумент envp задаёт окружение */
        /* первый аргумент списка аргументов (argv0) обычно равен имени исполняемого файла (по соглашению). */

        result = execle("/bin/cat", "cat", argv[1], (char *)NULL, envp);
        /* execle возвращает только при ошибке */
        if (result == -1) {
            perror("execle");
            exit(EXIT_FAILURE);
        }
    }

    return 0; /* недостижимо при успешном exec */
}
