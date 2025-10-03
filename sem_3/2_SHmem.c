/* Программа 2 для чтения текста исходного файла из разделяемой памяти.*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

/* Предполагаем, что размер исходного файла < SIZE */
const int kMaxSize =    65535;
const int kFullAccess = 0666;

int main()
{
    /* Указатель на разделяемую память */
    char* sh_memory = 0;
    /* Имя файла, использующееся для генерации ключа.
      Файл с таким именем должен существовать в текущей директории */
    const char* pathname = "QQQ.Q";

    /* Генерируем IPC ключ из имени файла  в текущей директории
       и номера экземпляра области разделяемой памяти 0 */
    const key_t key = ftok(pathname, 0);

    if (key < 0) {
        perror("Can't generate key\n");

        exit(EXIT_FAILURE);
    }

    /* Пытаемся найти разделяемую память по сгенерированному ключу */
    int shmid = shmget(key, kMaxSize, kFullAccess | IPC_CREAT);
    if (shmid < 0)
    {
        perror("Can't create shared memory\n");

        exit(EXIT_FAILURE);
    }

    /* Пытаемся отобразить разделяемую память в адресное пространство текущего процесса */
    sh_memory = shmat(shmid, NULL, 0);
    if (sh_memory == (void*)(-1))
    {
        perror("Can't attach shared memory\n");

        exit(EXIT_FAILURE);
    }

    /* Печатаем содержимое разделяемой памяти */
    printf ("%s\n", sh_memory);

    /* Отсоединяем разделяемую память и завершаем работу */
    int shmdet_res = shmdt(sh_memory);
    if(shmdet_res < 0)
    {
        perror("Can't detach shared memory\n");

        exit(EXIT_FAILURE);
    }

    /* Удаляем разделяемую память из системы */
    (void)shmctl(shmid, IPC_RMID, (struct shmid_ds *)NULL);

    return 0;
}
