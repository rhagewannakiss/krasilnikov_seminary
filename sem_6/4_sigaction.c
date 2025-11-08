#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t sig_occured = 0;

void sig_handler(int snum)
{
    (void)snum;
    sig_occured = 1;
}

int main(void)
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = &sig_handler;
    act.sa_flags = 0;

    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction()");
        return 1;
    }

    int count = 0;

    printf("Нажимайте Ctrl-C. После 5-и нажатий поведение SIGINT будет восстановлено.\n");

    while (1) {
        if (sig_occured) {
            sig_occured = 0;
            count++;
            fprintf(stderr, "signal... (%d)\n", count);
            if (count >= 5) {
                /* восстанавливаем поведение по умолчанию */
                struct sigaction def;
                sigemptyset(&def.sa_mask);
                def.sa_handler = SIG_DFL;
                def.sa_flags = 0;
                if (sigaction(SIGINT, &def, NULL) == -1) {
                    perror("sigaction(SIG_DFL)");
                    return 1;
                }
                printf("Поведение SIGINT восстановлено в SIG_DFL. Следующий Ctrl-C завершит программу.\n");
                /* теперь можно выйти из цикла или ждать следующего сигнала, чтобы программа завершилась по SIGINT */
                break;
            }
        }
        /* небольшая пауза, чтобы не крутить busy-loop */
        sleep(1);
    }

    /* ждём сигнала по умолчанию (Ctrl-C) или просто завершаем */
    printf("Программа завершает main и ожидает SIGINT (Ctrl-C) для фактического завершения, или можно завершить вручную.\n");
    /* можно либо ждать pause(), либо завершить */
    pause(); /* будет прерываться SIGINT и завершит программу (по SIG_DFL) */

    return 0;
}
