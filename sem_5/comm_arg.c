#include <stdlib.h>
#include <stdio.h>

/* envp — массив строк, завершающийся NULL. */

int main(int argc, char *argv[], char *envp[])
{
    int i;

    /* печатаем переменные окружения: пробегаем массив envp, пока не встретим NULL */
    for (i = 0; envp != NULL && envp[i] != NULL; i++) {
        printf("envp[%d]: %s\n", i, envp[i]);
    }
    /* envp может быть NULL на некоторых необычных реализациях,
       поэтому проверка envp != NULL безопасна. */

    /* печатаем аргументы командной строки: argc даёт их число,
       argv[0] — имя вызываемой программы. */
    for (i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    return 0;
}
