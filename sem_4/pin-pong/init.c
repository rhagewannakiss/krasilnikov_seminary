/* init.c -- инициализация: создаёт shm (int) и набор из 2 семафоров.
   Usage: ./init <start_value>
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>

#define SHM_KEY 3007
#define SEM_KEY 3007
#define SHM_SIZE sizeof(int)

/* union semun для semctl */
union semun { int val; struct semid_ds *buf; unsigned short *array; };

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <start_value>\n", argv[0]);
        return 1;
    }

    int start = atoi(argv[1]); /* начальное число для вычитания */

    /* создаём сегмент разделяемой памяти для одного int.
       IPC_CREAT — создать, если его нет; права 0600 — rw для владельца */
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0600);
    if (shm_id == -1) { perror("shmget"); return 1; }

    /* прикрепляемся и записываем начальное значение */
    int *p = (int*) shmat(shm_id, NULL, 0);
    if (p == (int*) -1) { perror("shmat"); return 1; }
    *p = start;
    if (shmdt(p) == -1) { perror("shmdt"); }

    /* создаём набор из 2 семафоров:
       сем[0] — даётся первому процессу (id=0) чтобы он начал
       сем[1] — изначально 0, второй процесс ждёт */
    int sem_id = semget(SEM_KEY, 2, IPC_CREAT | 0600);
    if (sem_id == -1) { perror("semget"); return 1; }

    unsigned short vals[2];
    vals[0] = 1; /* первый процесс стартует */
    vals[1] = 0; /* второй ждёт */
    union semun arg;
    arg.array = vals;
    if (semctl(sem_id, 0, SETALL, arg) == -1) { perror("semctl(SETALL)"); return 1; }

    printf("Initialized shared int = %d, shm_id=%d, sem_id=%d\n", start, shm_id, sem_id);
    return 0;
}
