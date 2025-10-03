/* разобраться как работает, написать комментарии,
   в том числе ко всем параметрам. */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/shm.h>

int main(const int argc, const char** argv) { //аргументы командонй строки - кол-во аргументов
                          //(всегда >= 1, т.к первый аргумент - имя прогарммы), и указатель на массив указателей на строки
    if (argc < 2) {
        fprintf(stderr, "Too few arguments, usage: %s <shmid>\n", argv[0]);

        return EXIT_FAILURE;
    }

    int   shmem_id = atoi(argv[1]);  //преобразуем строку в число
    fprintf(stderr, "%d\n", shmem_id);
    char* shmem =    shmat(shmem_id, // присоединяет сегмент памяти к адресному пр-ву процесса, принимает идентификатор процесса
                        NULL, // ядро самостоятельно выберет адрес
                        0);// флаги по умолчанию

    if (shmem == (void*)(-1)) {
        perror("shmat cant attach memory");

        return EXIT_FAILURE;
    }

    void *buf = shmat(shmem_id, NULL, 0);
    if (buf == (void *) -1) {
        perror("shmat");

        return EXIT_FAILURE;
    }

    printf("%s\n", (char*)buf);

     if (shmdt(shmem) == -1) {  //открепляем shared memory сегмент
        perror("shmdt");
        return EXIT_FAILURE;
    }

    return 0;
}
