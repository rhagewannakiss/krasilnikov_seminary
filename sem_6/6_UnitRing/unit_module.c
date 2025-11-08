#define _POSIX_C_SOURCE 200809L

#include "unit_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t got = 0;

void handler(int s) { (void)s; got = 1; }

/* простые P/V для System V семафора (mutex) */
int sem_p(int semid) {
    struct sembuf op = {0, -1, 0};
    return semop(semid, &op, 1);
}
int sem_v(int semid) {
    struct sembuf op = {0, 1, 0};
    return semop(semid, &op, 1);
}

int main(void)
{
    int shm_id = shmget(SHM_KEY, sizeof(struct ring_shm), 0);
    if (shm_id == -1) { perror("shmget"); return 1; }
    struct ring_shm *shm = shmat(shm_id, NULL, 0);
    if (shm == (void*)-1) { perror("shmat"); return 1; }

    int sem_id = semget(SEM_KEY, 1, 0);
    if (sem_id == -1) { perror("semget"); return 1; }

    /* регистрируем PID под mutex */
    if (sem_p(sem_id) == -1) { perror("sem_p"); return 1; }
    int slot = -1;
    for (int i = 0; i < MAX_MODULES; ++i) {
        if (shm->pids[i] == 0) { shm->pids[i] = getpid(); slot = i; shm->count++; break; }
    }
    sem_v(sem_id);

    if (slot == -1) {
        fprintf(stderr, "No free slot for module registration\n");
        return 1;
    }

    printf("Module started. PID=%d registered at slot %d\n", (int)getpid(), slot);

    /* установка обработчика сигнала */
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    /* основной цикл: ждём сигналов и при каждом сигнале инкрементируем value и печатаем */
    while (1) {
        pause(); /* ожидаем сигнал */
        if (got) {
            got = 0;
            /* делаем критическую секцию для изменения общего счётчика */
            if (sem_p(sem_id) == -1) { perror("sem_p"); break; }
            shm->value += 1;
            long v = shm->value;
            sem_v(sem_id);

            printf("Module PID=%d incremented value -> %ld\n", (int)getpid(), v);
            fflush(stdout);
        }
    }

    /* при завершении (в этом простом примере не обрабатываем SIGTERM/exit),
       освободим слот (но обычно нужно перехватывать сигналы завершения). */
    if (sem_p(sem_id) == -1) perror("sem_p");
    shm->pids[slot] = 0;
    shm->count--;
    sem_v(sem_id);

    shmdt(shm);
    return 0;
}
