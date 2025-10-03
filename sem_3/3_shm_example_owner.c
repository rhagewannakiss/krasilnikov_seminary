/* Проверить совместную работу с 4.
   Написать комментарии, В ТОМ ЧИСЛЕ К ПАРАМЕТРАМ!*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

const size_t kShmemSize = 4096;
const char* kShmemMessage = "Poglad kota\n";

int main(void) {
    int shmem_id = shmget(IPC_PRIVATE, // тип key_t, если key равно этому специальному значению,
                                              // то системный вызов игнорирует всё кроме 9-ти младших  битов  shmflg  и  создаёт  новый  общий сегмент памяти.
                                  kShmemSize, // shared memory size
                                  IPC_CREAT // создаёт новый сегмент
                                  | IPC_EXCL // гарантия нормального завершения системного вызова в случае, если сегмент ранее не существовал, иначе - ошибка
                                  | 0600    // права на запись и чтение, частный доступ (owner)
                                 );
    if (shmem_id < 0) {
        perror("shmget() error, cant get shared memory");

        return EXIT_FAILURE;
    }

    char* shmem = shmat(shmem_id, // присоединяет сегмент памяти к адресному пр-ву процесса, принимает идентификатор процесса
                                   NULL, // ядро самостоятельно выберет адрес
                                   0 // флаги по умолчанию(?) (т.к. можно задать SHM_RND/SHM_RDONLY
                                   // первый из них означает, что адрес shmaddr следует округлить до некоторй системно-зависимой величины.
                                   //SHM_RDONLY предписывает присоединить сегмент только для чтения; если он не установлен, присоединенный сегмент будет доступен и на чтение, и на запись
                                  );
    if (shmem == (void*)(-1)) {
        perror("Cant attach memory");

        return EXIT_FAILURE;
    }


    struct shmid_ds shmem_buff = {};
    int shmem_stat = shmctl(shmem_id, IPC_STAT, &shmem_buff); //получение статуса сегмента памяти
                                        //struct shmid_ds содержит shm_segsz (размер), shm_nattch (число присоединений) и т.д.
    if (shmem_stat < 0) {
        perror("Cant get stat of shared memory");
        shmdt(shmem);           // открепляет сегмент shared memory

        return EXIT_FAILURE;
    }

    // Проверка, что наш сегмент достаточно велик для сообщения
    size_t segment_size = shmem_buff.shm_segsz;
    if (segment_size < strlen(kShmemMessage)) {
        fprintf(stderr, "Not enough memory for message, error: segsize = %zu\n", segment_size);
        shmdt(shmem);               // открепляет` сегмент shared memory

        return EXIT_FAILURE;
    }

    strcpy((char*)shmem, kShmemMessage);  //копируем сообщение в сегмент shared memory

    printf("ID: %d\n", shmem_id);

    printf ("Press <Enter> to exit...");
    fgetc (stdin);

    shmdt(shmem);               // открепляет сегмент shared memory
    shmctl(shmem_id,
           IPC_RMID, // помечает сегмент памяти который будет удален
           NULL     // ядро самостоятельно выберет адрес
          );

    return 0;
}
