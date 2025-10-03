/* Программа, осуществляющая двунаправную связь через pipe
между процессом-родителем и процессом-ребенком */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main()
{
    int fd0[2], fd1[2], result;
    ssize_t size;
    char resstring[80];

    /* Создаем два pip'а */
    if (pipe(fd0) < 0)
    {
        perror("pipe fd0");
        exit(EXIT_FAILURE);
    }

    if (pipe(fd1) < 0)
    {
        perror("pipe fd1");
        exit(EXIT_FAILURE);
    }

    /* Порождаем новый процесс */
    result = fork();

    if (result <0)
    {
        /* Если создать процесс не удалось, сообщаем обэтом и завершаем работу */
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (result > 0)
    {
        /* Мы находимся в родительском процессе. Закрываем ненужные потоки данных */
        close(fd0[0]);
        close(fd1[1]);
        /* Пишем в первый pipe и читаем из второго */
        size = write(fd0[1], "Привет, дитя!", (ssize_t)sizeof("Привет, дитя!"));
        if (size != (ssize_t)sizeof("Привет, дитя!"))
        {
            errno = EIO;
            perror("Parent: can't write all string");
            exit(EXIT_FAILURE);
        }
        size = read(fd1[0], resstring, (ssize_t)(sizeof(resstring) - 1));

        if (size < 0)
        {
            perror("Parent: read");
            exit(EXIT_FAILURE);
        }
        resstring[size] = '\0';
        printf("Parent: %s\n", resstring);

        close(fd0[1]);
        close(fd1[0]);
        printf("Parent exit\n");
    }
    else
    {
        /* Мы находимся в порожденном процессе. Закрываем ненужные потоки данных */
        close(fd0[1]);
        close(fd1[0]);
        /* Читаем из первого pip'а и пишем во второй */
        size = read(fd0[0], resstring, (ssize_t)(sizeof(resstring) - 1));
        if (size < 0)
        {
            perror("Child: read");
            exit(EXIT_FAILURE);
        }

        resstring[size] = '\0';
        printf("Child: %s\n",resstring);
        size = write(fd1[1], "Ну привет, папик!", (ssize_t)sizeof("Ну привет, папик!"));
        if (size != (ssize_t)sizeof("Ну привет, папик!"))
        {
            errno = EIO;
            perror("Child: can't write all string");
            exit(EXIT_FAILURE);
        }

        close(fd0[0]);
        close(fd1[1]);
    }

    return 0;
}