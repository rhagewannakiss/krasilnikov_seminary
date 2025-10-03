/* Программа для записи текста исходного файла в разделяемую память,
   проверить совместнос 2 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

 /* Предполагаем, что размер исходного файла < SIZE */
const size_t kMaxSize =    65535;
const int    kFullAccess = 0666;
const int    kChunkSize =  1024;

int main() {
    /* Имя файла, использующееся для генерации ключа и чтения исходного текста.
    Файл с таким именем должен существовать в текущей директории */
    const char* pathname = "QQQ.Q";

    int     file_descr =   0;
    ssize_t n_bytes_read = 0;
    size_t  read_data =    0;

    /* Генерируем IPC ключ из имени файла и номера экземпляра области разделяемой памяти 0 */
    const key_t key = ftok(pathname, 0);
    if(key < 0) {
        perror("Can't generate key\n");

        exit(EXIT_FAILURE);
    }

    /* Пытаемся создать разделяемую память для сгенерированного ключа */
    /* IPC дескриптор для области разделяемой памяти */
    int shmid = shmget(key, kMaxSize, kFullAccess | IPC_CREAT);
    if (shmid < 0) {
        perror("Can\'t create shared memory\n");

        exit(EXIT_FAILURE);
    }

    /* Пытаемся отобразить разделяемую память в адресное пространство текущего процесса */
    char* sh_memory = (char*)shmat(shmid, NULL, 0);
    if (sh_memory == (void*)(-1)) {
        perror("Can't attach shared memory\n");

        exit(EXIT_FAILURE);
    }

    /* Открываем файл только на чтение*/
    if((file_descr = open(pathname, O_RDONLY)) < 0) {
        (void)shmdt(sh_memory);
        perror("Can't open source file\n");

        exit(EXIT_FAILURE);
    }

    /* Читаем файл порциями по 1kb до тех пор, пока не достигнем конца файла
    или не возникнет ошибка */

    char* current_ptr =  sh_memory;
    while((n_bytes_read = read(file_descr, current_ptr, kChunkSize)) > 0) {
        read_data += (size_t)n_bytes_read;
        current_ptr += n_bytes_read;
    }

    /* Закрываем файл */
    close(file_descr);

    /* Если возникла ошибка - завершаем работу */
    if(n_bytes_read < 0) {
        perror("Error reading source file\n");
        (void)shmdt(sh_memory);

        exit(EXIT_FAILURE);
    }

    /* После всего считанного текста вставляем признак конца строки,
    чтобы впоследствии распечатать все одним printf'ом */
    *current_ptr = '\0';

    /* Печатаем содержимое буфера.*/
    printf("%s\n", current_ptr);

    /* Отсоединяем разделяемую память и завершаем работу */
    if(shmdt(sh_memory) < 0) {
        perror("Can't detach shared memory\n");

        exit(EXIT_FAILURE);
    }

    return 0;
}
