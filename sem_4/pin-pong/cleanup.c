/* cleanup.c -- удалить shm и семафоры */
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 3007
#define SEM_KEY 3007

int main(void) {
    int shm_id = shmget(SHM_KEY, 0, 0); /* 0 size — просто получить id */
    if (shm_id != -1) {
        shmctl(shm_id, IPC_RMID, NULL);
        printf("Removed shm id %d\n", shm_id);
    }
    int sem_id = semget(SEM_KEY, 0, 0);
    if (sem_id != -1) {
        semctl(sem_id, 0, IPC_RMID);
        printf("Removed sem id %d\n", sem_id);
    }
    return 0;
}