/* worker.c -- worker process: запускается с аргументом 0 или 1
   Usage: ./worker 0
          ./worker 1
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define SHM_KEY 3007
#define SEM_KEY 3007
#define SHM_SIZE sizeof(int)

/* обёртки для P/V операций (упрощают чтение) */
int sem_p(int semid, int semnum) {
    struct sembuf op;
    op.sem_num = semnum;
    op.sem_op = -1; /* P */
    op.sem_flg = 0;
    return semop(semid, &op, 1);
}

int sem_v(int semid, int semnum) {
    struct sembuf op;
    op.sem_num = semnum;
    op.sem_op = 1; /* V */
    op.sem_flg = 0;
    return semop(semid, &op, 1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <id: 0 or 1>\n", argv[0]);
        return 1;
    }
    int id = atoi(argv[1]);
    if (id != 0 && id != 1) {
        fprintf(stderr, "id must be 0 or 1\n");
        return 1;
    }
    int other = 1 - id; /* индекс семафора второго процесса */

    /* подключаемся к уже созданным ресурсам (инициализатор должен их создать) */
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0600);
    if (shm_id == -1) { perror("shmget"); return 1; }
    int *p = (int*) shmat(shm_id, NULL, 0);
    if (p == (int*) -1) { perror("shmat"); return 1; }

    int sem_id = semget(SEM_KEY, 2, 0600);
    if (sem_id == -1) { perror("semget"); return 1; }

    while (1) {
        /* ждём своей очереди — P на своём семафоре
           это обеспечит строгую последовательность: сначала id=0 (если сем[0]=1), потом id=1 и т.д. */
        if (sem_p(sem_id, id) == -1) {
            perror("sem_p");
            break;
        }

        /* критическая секция: читаем и уменьшаем общее число */
        if (*p <= 0) {
            /* число уже 0 — передаём токен следующему, чтобы он тоже смог корректно выйти,
               и прерываем цикл. Это позволяет избежать взаимной блокировки */
            if (sem_v(sem_id, other) == -1) perror("sem_v");
            break;
        }

        int newval = (*p) - 1;
        *p = newval;
        printf("PID %d (id=%d) decremented value -> %d\n", getpid(), id, newval);
        fflush(stdout);

        sleep(1); /* задержка для наглядности (можно убрать в реальном приложении) */

        /* передаём управление другому процессу через V на его семафоре */
        if (sem_v(sem_id, other) == -1) {
            perror("sem_v");
            break;
        }
    }

    /* отсоединяемся от разделяемой памяти */
    if (shmdt(p) == -1) perror("shmdt");
    return 0;
}
