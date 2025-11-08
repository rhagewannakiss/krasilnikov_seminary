
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int year;
    if (argc < 2) {
        fprintf(stderr, "child: too few arguments\n");
        return 2;
    }

    year = atoi(argv[1]);
    if (year <= 0)
        return 2;

    /* правило високосного года:
       - год кратен 400 => високосный
       - иначе, если кратен 100 => не високосный
       - иначе, если кратен 4 => високосный
       (эквивалент: (year%4==0 && year%100!=0) || year%400==0) ) */

    if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) {
        /* отправляем SIGUSR1 родителю */
        if (kill(getppid(), SIGUSR1) == -1) {
            perror("kill(SIGUSR1)");
            return 1;
        }
    } else {
        if (kill(getppid(), SIGUSR2) == -1) {
            perror("kill(SIGUSR2)");
            return 1;
        }
    }

    return 0;
}
