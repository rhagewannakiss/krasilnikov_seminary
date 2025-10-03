/* Программа для записи в FIFO*/
/* Для отладки использовать утилиту strace: strace -e trace=open,read ./имя программы */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main()
{
    int fd;
    ssize_t size;
    char name[]="aaa.fifo";
    const char *msg = "Погладь Кота!";

    /* Обнуляем маску создания файлов текущего процесса для того, чтобы
    права доступа у создаваемого FIFO точно соотвествовали параметру вызова mknod() */
    (void)umask(0);

    /* Попытаемся создать FIFO с именем  aaa.fifo в текущей директории,
    если FIFO уже существует, то этот if нужно удалить !!! или добавить анализ.  */
    if (mkfifo(name, 0666) < 0)
    {
        if (errno != EEXIST)
        {
            /* Если создать FIFO не удалось, печатаем об этом сообщение и прекращаем работу */
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
        /* EEXIST — продолжаем работу */
    }

    /* Открываем FIFO на запись. */
    if ((fd = open(name, O_WRONLY)) < 0)
    {
        /* Если открыть FIFO не удалось, печатаем об этом сообщение и прекращаем работу */
        perror("open (O_WRONLY)");
        exit(EXIT_FAILURE);
    }

    /* Записываем строку (без явного '\0') */
    size = write(fd, msg, (ssize_t)strlen(msg));

    if (size < 0)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (size != (ssize_t)strlen(msg))
    {
        errno = EIO;
        perror("Can't write all string to FIFO");
        close(fd);
        exit(EXIT_FAILURE);
    }

    /* Успех — напечатаем короткое сообщение */
    printf("Writer: wrote %zd bytes to %s\n", size, name);

    close(fd);
    return 0;
}