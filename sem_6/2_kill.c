#include <sys/types.h>  /* содержит typedef pid_t на POSIX */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* getpid */

int main(void)
{
    pid_t dpid = getpid(); /* посылаем сигнал самому себе */

    printf("PID = %d. Устанавливаем игнорирование SIGABRT и посылаем SIGABRT себе.\n", (int)dpid);

    /* сделать игнорирование сигнала */
    if (signal(SIGABRT, SIG_IGN) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    /* теперь посылаем сигнал самому себе.
       kill возвращает 0 при успехе, -1 при ошибке. */
    if (kill(dpid, SIGABRT) == -1) {
        perror("kill");
        return 1;
    }

    printf("SIGABRT отправлен, но проигнорирован; процесс продолжает работу.\n");

    return 0;
}
