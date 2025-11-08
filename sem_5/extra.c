#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h> /* flock */
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <shared-file>\n", argv[0]);
        return 1;
    }
    const char *fname = argv[1];

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* порожденный: Writer — читает stdin и дописывает в файл (append) */
        int fd;
        char line[512];

        fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) { perror("open writer"); return 1; }

        printf("Writer started (pid=%d). Type lines and press Enter to append to %s\n", (int)getpid(), fname);
        while (fgets(line, sizeof(line), stdin)) {
            size_t len = strlen(line);
            /* блокируем файл при записи, чтобы не перемешивать записи других писателей */
            if (flock(fd, LOCK_EX) == -1) {
                perror("flock LOCK_EX");
            }
            ssize_t off = 0;
            while (off < (ssize_t)len) {
                ssize_t w = write(fd, line + off, len - off);
                if (w < 0) { perror("write"); break; }
                off += w;
            }
            /* сбросим содержимое на диск (опционально) */
            fsync(fd);
            if (flock(fd, LOCK_UN) == -1) {
                perror("flock LOCK_UN");
            }
        }

        close(fd);
        return 0;
    } else {
        /* родитель: Reader — следит за файлом и печатает добавленные данные (простая реализация tail) */
        int fd;
        struct stat st;
        off_t pos = 0;
        char buf[512];

        /* открываем для чтения */
        fd = open(fname, O_RDONLY | O_CREAT, 0644);
        if (fd < 0) { perror("open reader"); return 1; }

        /* сначала переместимся в конец — будем читать только добавления */
        pos = lseek(fd, 0, SEEK_END);

        printf("Reader started (pid=%d). Waiting for new data in %s\n", (int)getpid(), fname);

        while (1) {
            /* периодически проверяем размер файла и читаем новые данные */
            if (fstat(fd, &st) == -1) {
                perror("fstat");
                break;
            }
            if (st.st_size > pos) {
                /* появились новые данные */
                ssize_t toread = (ssize_t)(st.st_size - pos);
                ssize_t r = read(fd, buf, (toread < (ssize_t)sizeof(buf) ? toread : (ssize_t)sizeof(buf)));
                if (r > 0) {
                    /* печатаем и обновляем позицию */
                    fwrite(buf, 1, r, stdout);
                    fflush(stdout);
                    pos += r;
                } else if (r == 0) {
                    /* ничего */
                } else {
                    perror("read");
                    break;
                }
            }
            sleep(1); /* задержка опроса — на лабораторный пример достаточно */
        }

        close(fd);
        return 0;
    }
}
