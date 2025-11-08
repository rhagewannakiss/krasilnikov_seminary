#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

sig_atomic_t sig_status = 0;

void handle_usr1(int s_num) { (void)s_num; sig_status = 1; }
void handle_usr2(int s_num) { (void)s_num; sig_status = 2; }

int main(int argc, char **argv)
{
    struct sigaction act_usr1, act_usr2;

    sigemptyset(&act_usr1.sa_mask);
    sigemptyset(&act_usr2.sa_mask);
    act_usr1.sa_flags = 0;
    act_usr2.sa_flags = 0;
    act_usr1.sa_handler = &handle_usr1;
    act_usr2.sa_handler = &handle_usr2;

    if (sigaction(SIGUSR1, &act_usr1, NULL) == -1) {
        perror("sigaction (act_usr1)");
        return 1;
    }

    /* РЕ: в исходнике была опечатка: передавали act_usr1 вторым вызовом.
       Здесь корректно регистрируем обработчик для SIGUSR2. */
    if (sigaction(SIGUSR2, &act_usr2, NULL) == -1) {
        perror("sigaction (act_usr2)");
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }

    pid_t c = fork();
    if (c < 0) {
        perror("fork");
        return 1;
    }

    if (c == 0) {
        /* дочерний: заменяемся на 3_signal-child */
        /* предположим, что исполняемый файл называется ./3_signal-child */
        execl("./3_signal-child", "./3_signal-child", argv[1], (char *)NULL);
        /* если exec вернётся — ошибка */
        perror("execl");
        _exit(1);
    }

    /* родитель: ждем сигнала. Используем простой цикл, безопасно проверяя sig_status.
       можно оптимизировать через pause() и проверку, но для простоты используем sleep. */
    while (1) {
        if (sig_status == 1) {
            printf("%s: leap year\n", argv[1]);
            break;
        }
        if (sig_status == 2) {
            printf("%s: not leap year\n", argv[1]);
            break;
        }
        sleep(1);
    }

    /* опционально дождёмся завершения дочернего, чтобы не оставлять зомби */
    waitpid(c, NULL, 0);

    return 0;
}
