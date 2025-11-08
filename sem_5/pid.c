#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    pid_t pid = getpid();     /* идентификатор текущего процесса */
    pid_t ppid = getppid();   /* идентификатор родительского процесса */

    /* pid_t не имеет стандартного спецификатора в printf, поэтому безопасно кастовать:
       здесь используем (int) — обычно pid_t помещается в int, но можно кастовать в long и
       печатать через %ld при необходимости. */
    printf("My pid = %d, my ppid = %d\n", (int)pid, (int)ppid);

    return 0;
}
