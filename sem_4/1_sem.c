#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

/* простой P (wait) на семафоре 0 в наборе, полученном по ftok(pathname, 0)*/

int main(int argc, char *argv[], char *envp[])
{
    int semid;
    char pathname[] = "1_sem.c"; /* файл, используемый ftok() — должен существовать в fs
                                   ftok генерирует ключ на базе inode + project id (второй аргумент) */
    key_t key;
    struct sembuf mybuf;

    /* получаем ключ для System V IPC (одинаковый ключ дадут все процессы,
       которые вызовут ftok с этим pathname и одинаковым proj_id) */
    key = ftok(pathname, 0);
    if (key == (key_t)-1) {
        /* perror печатает понятное сообщение по errno */
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    /* semget: либо создаёт, либо возвращает id уже существующего набора семафоров
       параметр 1 — количество семафоров в наборе
       флаги: IPC_CREAT — создать, если нет; 0666 — права доступа (rw для всех) */
    if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    /* подготовка операции семафора:
       sem_num — индекс семафора в наборе (0 поскольку у нас один семафор),
       sem_op  = -1 — операция P (wait): уменьшить значение на 1; если результат будет <0 — блокируемся
       sem_flg = 0 — блокирующее поведение, не используем SEM_UNDO и т.п. */
    mybuf.sem_num = 0;
    mybuf.sem_op  = -1;
    mybuf.sem_flg = 0;

    /* semop выполняет одну или несколько операций атомарно. Здесь — одна операция
       если значение семафора было 0, вызов будет блокирующим до тех пор, пока другой процесс
       не выполнит V (sem_op = +1) */
    if (semop(semid, &mybuf, 1) < 0) {
        perror("semop (wait)");
        exit(EXIT_FAILURE);
    }

    /* если мы сюда попали — P прошёл успешно */
    printf("The condition is present (P succeeded)\n");
    return 0;
}
