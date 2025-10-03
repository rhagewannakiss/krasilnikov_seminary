/* Программа для чтения из  FIFO */
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
    /* Увеличен буфер чтобы избежать обрезания UTF-8 последовательностей */
    char resstring[128];
    char name[]="aaa.fifo";

    /* Открываем FIFO на чтение.*/
    if ((fd = open(name, O_RDONLY)) < 0)
    {
        /* Если открыть FIFO не удалось,
        печатаем об этом сообщение и прекращаем работу */
        perror("open (O_RDONLY)");
        exit(EXIT_FAILURE);
    }

    /* Пробуем прочитать из FIFO (оставим 1 байт для '\0') */
    size = read(fd, resstring, (ssize_t)(sizeof(resstring) - 1));
    if (size < 0)
    {
        /* Если прочитать не смогли, сообщаем об ошибке и завершаем работу */
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    /* Нуль-терминируем прочитанное */
    resstring[size] = '\0';

    /* Печатаем прочитанную строку */
    printf("Resstring: %s\n", resstring);

    /* close(fd) нужен, чтобы освободить дескриптор файлового дескриптора.
    закрытие чтения/записи на FIFO влияет на другой конец: когда все писатели закрыли FIFO, читатель получит read() == 0 (EOF);
    аналогично для писателя — при закрытии всех читателей write может завершиться с ошибкой или сигналом SIGPIPE.*/

    close(fd);
    return 0;
}