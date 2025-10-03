/* Третья программа (клиент) читает с терминала число и пихает его в трубу.*/
/* Если ввести не число, то клиент и сервер завершают работу.*/
/* Клиент также читает ответ из второй трубы (обратная связь) и печатает результат. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main()
{
    const char *in_fifo = "to_server.fifo";   /* пишем сюда */
    const char *out_fifo = "to_client.fifo";  /* читаем отсюда */
    int in_fd = -1, out_fd = -1;
    char buf[128];

    /* Открываем FIFO для записи (to_server) */
    if ((in_fd = open(in_fifo, O_WRONLY)) < 0)
    {
        perror("open to_server.fifo");
        return 1;
    }

    /* Открываем FIFO для чтения ответов (to_client) */
    if ((out_fd = open(out_fifo, O_RDONLY)) < 0)
    {
        perror("open to_client.fifo");
        close(in_fd);
        return 1;
    }

    printf("Введите целое число (или любой нечисловой ввод для выхода):\n");
    while (fgets(buf, sizeof(buf), stdin))
    {
        /* Уберём перевод строки */
        char *nl = strchr(buf, '\n');
        if (nl) *nl = '\0';

        /* Проверим - число ли это */
        char *endptr;
        long val = strtol(buf, &endptr, 10);
        if (endptr == buf || *endptr != '\0')
        {
            /* Если ввести не число, то клиент и сервер завершают работу. */
            printf("Non-number input: exiting client.\n");
            break;
        }

        /* Отправляем на сервер (добавим '\n' чтобы серверу было удобно читать) */
        char sendbuf[64];
        int sl = snprintf(sendbuf, sizeof(sendbuf), "%ld\n", val);
        if (write(in_fd, sendbuf, (size_t)sl) != sl)
        {
            perror("write to server fifo");
            break;
        }

        /* Читаем ответ сервера */
        char reply[128];
        ssize_t rr = read(out_fd, reply, sizeof(reply) - 1);
        if (rr < 0)
        {
            perror("read from server fifo");
            break;
        }
        if (rr == 0)
        {
            printf("Server closed connection.\n");
            break;
        }
        reply[rr] = '\0';
        printf("Result from server: %s", reply);
    }

    close(in_fd);
    close(out_fd);
    return 0;
}