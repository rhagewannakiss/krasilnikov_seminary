#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>

#define SHMEM_SIZE 4096
#define SH_MESSAGE "Hello World!\n"

#define SEM_KEY 2007
#define SHM_KEY 2007

/* union semun обязателен для semctl в приложении (man 2 semctl примечает это) */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(void)
{
    int shm_id, sem_id;
    char *shm_buf;
    struct shmid_ds ds;
    struct sembuf sb[1];
    unsigned short sem_vals[1];
    union semun sem_arg;

    /* 1) создать сегмент разделяемой памяти
       IPC_CREAT — создать, если отсутствует
       флаг IPC_EXCL можно использовать, чтобы получить ошибку если уже есть (зависит от сценария)
       Здесь оставляем только IPC_CREAT, чтобы была возможность повторно получить существующий сегмент */
    shm_id = shmget(SHM_KEY, SHMEM_SIZE, IPC_CREAT | 0600);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }

    /* 2) создать/получить набор из одного семафора */
    sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0600);
    if (sem_id == -1) {
        perror("semget");
        return 1;
    }

    printf("Semaphore id: %d\n", sem_id);

    /* 3) инициализируем семафор: массив значений (тут только один семафор) */
    sem_vals[0] = 1;        /* ставим 1 — ресурс доступен */
    sem_arg.array = sem_vals;

    /* SETALL устанавливает значения всех семафоров набора в соответствии с array */
    if (semctl(sem_id, 0, SETALL, sem_arg) == -1) {
        perror("semctl(SETALL)");
        return 1;
    }

    /* 4) прикрепляем сегмент в адресное пространство процесса */
    shm_buf = (char *) shmat(shm_id, NULL, 0);
    if (shm_buf == (char *) -1) {
        perror("shmat");
        return 1;
    }

    /* 5) проверяем фактический размер сегмента через shmctl(IPC_STAT)
       (чтобы не переписать память, если выделенный сегмент меньше сообщения) */
    if (shmctl(shm_id, IPC_STAT, &ds) == -1) {
        perror("shmctl(IPC_STAT)");
        return 1;
    }

    if ((int)ds.shm_segsz < (int)strlen(SH_MESSAGE) + 1) {
        fprintf(stderr, "error: segsize=%ld\n", (long)ds.shm_segsz);
        return 1;
    }

    /* 6) записываем сообщение в разделяемую память
       важно: записываем *в* shm_buf, а не наоборот (в коде было перепутано) */
    strcpy(shm_buf, SH_MESSAGE);

    printf("Wrote message to shared memory (id=%d)\n", shm_id);

    /* 7) демонстрация синхронизации: исполняем P — ждём, пока кто-то другой не выполнит V
       это пример: писатель записал и ждёт подтверждения от читателя */
    sb[0].sem_num = 0;
    sb[0].sem_flg = 0;
    sb[0].sem_op = -1; /* дождаться V от читателя */

    if (semop(sem_id, sb, 1) == -1) {
        /* если semop неудачен — печатаем ошибку, но попробуем всё равно очистить ресурсы ниже */
        perror("semop wait");
    }

    /* 8) удаляем семафор и сегмент (освобождаем ресурсы)
       semctl IPC_RMID — помечает набор семафоров для удаления (будет удалён после выхода всех процессов) */
    if (semctl(sem_id, 0, IPC_RMID, sem_arg) == -1) {
        perror("semctl(IPC_RMID)");
    }

    if (shmdt(shm_buf) == -1) {
        perror("shmdt");
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl(IPC_RMID)");
    }

    return 0;
}