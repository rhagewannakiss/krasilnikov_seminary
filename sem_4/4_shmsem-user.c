#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>

#define SEM_KEY 2007
#define SHM_KEY 2007
#define SHMEM_SIZE 4096

int main (int argc, char ** argv)
{
    int shm_id, sem_id;
    char *shm_buf;
    struct sembuf sb[1];

    /* 1) получаем id уже существующего сегмента разделяемой памяти
       взов без IPC_CREAT — если ресурса нет, будет ошибка */
    shm_id = shmget(SHM_KEY, SHMEM_SIZE, 0600);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    /* 2) получаем id набора семафоров (ожидаем, что он уже создан писателем)*/
    sem_id = semget(SEM_KEY, 1, 0600);
    if (sem_id == -1) {
        perror("semget");
        return 1;
    }

    /* 3) прикрепляем разделяемую память */
    shm_buf = (char *) shmat(shm_id, 0, 0);
    if (shm_buf == (char *) -1) {
        perror("shmat");
        return 1;
    }

    /* 4) читаем сообщение и печатаем его
       shm_buf — массив символов, поэтому корректно печатаем как строку */
    printf("Message: %s\n", shm_buf);

    /* 5) ыыполняем V (увеличиваем семафор), тем самым даём сигнал писателю,
       который может был заблокирован на P (sem_op = -1) */
    sb[0].sem_num = 0;
    sb[0].sem_flg = 0;
    sb[0].sem_op  = 1;

    if (semop(sem_id, sb, 1) == -1) {
        perror("semop (signal)");
    }

    /* 6) отсоединяем сегмент разделяемой памяти (сама память остаётся до IPC_RMID) */
    if (shmdt(shm_buf) == -1) {
        perror("shmdt");
    }

    return 0;
}
