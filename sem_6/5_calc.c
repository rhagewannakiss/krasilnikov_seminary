#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

volatile sig_atomic_t print_flag = 0;

void handler(int s) { (void)s; print_flag = 1; }

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    long N = atol(argv[1]);
    if (N < 0) N = 0;

    if (signal(SIGUSR1, handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    pid_t pid = getpid();
    printf("PID = %d. Посылайте SIGUSR1 (%s) из другого терминала: kill -USR1 %d\n",
           (int)pid, "SIGUSR1", (int)pid);

    unsigned long long fact = 1ULL;
    for (long i = 1; i <= N; ++i) {
        fact *= (unsigned long long)i;

        /* имитация долгой работы — чтобы можно было послать сигнал */
        sleep(1);

        if (print_flag) {
            print_flag = 0;
            time_t t = time(NULL);
            char buf[64];
            struct tm tm;
            localtime_r(&t, &tm);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
            printf("[signal] time=%s intermediate: i=%ld fact=%llu\n", buf, i, fact);
            fflush(stdout);
        }
    }

    printf("Final: %ld! = %llu\n", N, fact);
    return 0;
}
