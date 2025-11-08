#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int fd_in, fd_out;
    ssize_t nread;
    char buffer[256];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    /* открываем файл только для чтения */
    fd_in = open(argv[1], O_RDONLY);
    if (fd_in < 0) {
        perror("open input");
        exit(EXIT_FAILURE);
    }

    /* создаём выходной файл copy_of_<имя> */
    char out_name[512];
    snprintf(out_name, sizeof(out_name), "copy_of_%s", argv[1]);
    fd_out = open(out_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
        perror("open output");
        close(fd_in);
        exit(EXIT_FAILURE);
    }

    /* читаем и пишем порциями, пока read не вернёт 0 (EOF) */
    while ((nread = read(fd_in, buffer, sizeof(buffer))) > 0) {
        ssize_t nwritten = 0;
        ssize_t off = 0;
        /* обработка частичных записей: loop пока все байты не записаны */
        while (off < nread) {
            nwritten = write(fd_out, buffer + off, nread - off);
            if (nwritten < 0) {
                perror("write");
                close(fd_in);
                close(fd_out);
                exit(EXIT_FAILURE);
            }
            off += nwritten;
        }
    }

    if (nread < 0) {
        perror("read");
    } else {
        printf("Copied '%s' -> '%s'\n", argv[1], out_name);
    }

    /* закрываем файлы */
    if (close(fd_in) < 0) {
        perror("close input");
    }
    if (close(fd_out) < 0) {
        perror("close output");
    }

    return 0;
}
