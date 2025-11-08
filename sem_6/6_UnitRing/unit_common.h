/* unit_common.h */
#ifndef UNIT_COMMON_H
#define UNIT_COMMON_H

#include <sys/types.h>

#define SHM_KEY  6007
#define SEM_KEY  6007
#define MAX_MODULES 16

struct ring_shm {
    pid_t pids[MAX_MODULES]; /* 0 == свободно */
    long value;              /* общее число, которое модули увеличивают */
    int count;               /* число зарегистрированных модулей */
};

#endif /* UNIT_COMMON_H */
