#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

int main(int argc, char *argv[], char *envp[])
{
    int semid;
    char pathname[] = "2_sem.c"; /* файл для ftok; должен существовать */
    key_t key;
    struct sembuf mybuf;

    /* получаем ключ, соответствующий тому же файлу (чтобы читатель и писатель
       использовали один и тот же набор семафоров)*/
    key = ftok(pathname, 0);
    if (key == (key_t)-1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    /* получаем идентификатор набора семафоров (создаём, если отсутствует) */
    if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    /* подготовка операции: sem_op = +1 => V (signal) — увеличиваем значение семафора */
    mybuf.sem_num = 0;
    mybuf.sem_op  = 1;
    mybuf.sem_flg = 0;

    /* semop выполнит V — если кто-то ждал (P) он разблокируется и продолжит */
    if (semop(semid, &mybuf, 1) < 0) {
        perror("semop (signal)");
        exit(EXIT_FAILURE);
    }

    printf("Added 1 to semaphore (V succeeded)\n");
    return 0;
}
