/* Перая программа создаёт или удаляет трубу.*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *fifo1 = "to_server.fifo";
    const char *fifo2 = "to_client.fifo";

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s create|remove\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "create") == 0)
    {
        (void)umask(0);
        if (mkfifo(fifo1, 0666) < 0 && errno != EEXIST)
        {
            perror("mkfifo to_server");
            return 1;
        }
        if (mkfifo(fifo2, 0666) < 0 && errno != EEXIST)
        {
            perror("mkfifo to_client");
            return 1;
        }
        printf("FIFOs created (or already exist).\n");
    }
    else if (strcmp(argv[1], "remove") == 0)
    {
        if (unlink(fifo1) < 0 && errno != ENOENT) { perror("unlink to_server"); }
        if (unlink(fifo2) < 0 && errno != ENOENT) { perror("unlink to_client"); }
        printf("FIFOs removed (if existed).\n");
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}