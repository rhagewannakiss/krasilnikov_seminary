#include "unit_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>

/* union semun для semctl */
union semun { int val; struct semid_ds *buf; unsigned short *array; };

int main(void)
{
    int shm_id = shmget(SHM_KEY, sizeof(struct ring_shm), IPC_CREAT | 0600);
    if (shm_id == -1) { perror("shmget"); return 1; }
    struct ring_shm *shm = shmat(shm_id, NULL, 0);
    if (shm == (void*)-1) { perror("shmat"); return 1; }

    /* инициализация структуры (при первом создании) */
    shm->value = 0;
    shm->count = 0;
    for (int i = 0; i < MAX_MODULES; ++i) shm->pids[i] = 0;

    /* семафор как mutex (1 элемент) */
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0600);
    if (sem_id == -1) { perror("semget"); return 1; }

    unsigned short v = 1;
    union semun arg; arg.array = &v;
    semctl(sem_id, 0, SETVAL, arg);

    printf("Dispatcher started. shm_id=%d sem_id=%d\n", shm_id, sem_id);
    printf("Press Ctrl-C to stop dispatcher.\n");

    /* round-robin: просто пробегаем массив pids и посылаем SIGUSR1 по очереди */
    while (1) {
        for (int i = 0; i < MAX_MODULES; ++i) {
            pid_t pid = shm->pids[i];
            if (pid != 0) {
                /* Отправляем сигнал модулю */
                if (kill(pid, SIGUSR1) == -1) {
                    /* Если процесс не существует, очищаем слот */
                    perror("kill (maybe module died)");
                    shm->pids[i] = 0;
                } else {
                    /* даём небольшой интервал, чтобы модуль обработал сигнал и напечатал */
                    usleep(200000); /* 200 ms */
                }
            }
        }
        sleep(1); /* пауза между раундами */
    }

    /* никогда сюда не дойдём в простом примере; при необходимости очистите shm/sem вручную */
    return 0;
}
