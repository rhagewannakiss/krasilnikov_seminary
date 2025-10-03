/* Вторая программа(сервер) получает из трубы значение, возводит в квадрат
и печатает его.*/
/* Дополнительно: использую вторую трубу для обратной связи. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main()
{
    const char *in_fifo = "to_server.fifo";
    const char *out_fifo = "to_client.fifo";
    int in_fd = -1, out_fd = -1;
    char buf[128];

    /* Открываем входную FIFO на чтение (блокирует пока есть хотя бы один писатель) */
    if ((in_fd = open(in_fifo, O_RDONLY)) < 0)
    {
        perror("open in_fifo");
        return 1;
    }

    /* Открываем выходную FIFO на запись (блокирует пока есть читатель) */
    if ((out_fd = open(out_fifo, O_WRONLY)) < 0)
    {
        perror("open out_fifo");
        close(in_fd);
        return 1;
    }

    while (1)
    {
        ssize_t r = read(in_fd, buf, sizeof(buf) - 1);
        if (r < 0)
        {
            perror("read");
            break;
        }
        if (r == 0)
        {
            /* Конец потока (все писатели закрыли) — завершаемся. */
            break;
        }
        buf[r] = '\0';

        /* Уберём возможные перевод строки */
        char *nl = strchr(buf, '\n');
        if (nl) *nl = '\0';

        /* Попробуем распарсить число */
        char *endptr;
        long val = strtol(buf, &endptr, 10);
        if (endptr == buf || *endptr != '\0')
        {
            /* Если ввести не число, то клиент и сервер завершают работу. */
            printf("Server: received non-number, exiting.\n");
            break;
        }

        long sq = val * val;
        char outbuf[128];
        int len = snprintf(outbuf, sizeof(outbuf), "%ld\n", sq);
        if (write(out_fd, outbuf, (size_t)len) != len)
        {
            perror("write to out_fifo");
            break;
        }
    }

    close(in_fd);
    close(out_fd);
    return 0;
}