#define _GNU_SOURCE
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>

/* Константы времени компиляции */
#define K_SHM_KEY_BASE ((key_t)0xDEADBABE)
#define K_FULL_ACCESS 0777
#define K_PAGE_SIZE 4096U
#define MAX_TTY_LEN 64U
#define MAX_MSG_LEN 2048U   /* фиксированный размер буфера для сообщения */
#define K_DELAY_US (10 * 1000) /* 10 ms polling */

/* Структура в SHM:
   state: простой lock 0 == free, 1 == writing
   sender_tid: id терминала-отправителя
   msg_seq: монотонный номер сообщения (увеличивается при каждой записи)
   sender_tty: имя tty отправителя
   msg: NUL-terminated текст сообщения
*/
struct shm_area {
    volatile int32_t state;               /* lock: 0 свободен, 1 пишется */
    int32_t sender_tid;                   /* терминал отправителя */
    int32_t msg_seq;                      /* порядковый номер сообщения */
    char sender_tty[MAX_TTY_LEN];         /* имя tty отправителя */
    char msg[MAX_MSG_LEN];                /* само сообщение */
};

/* Глобальные для очистки */
static int g_shmid = -1;
static struct shm_area *g_shm = NULL;
static key_t g_key = 0;

/* Простейший CAS (GCC builtin) */
static inline int cas_int32(volatile int32_t *ptr, int32_t expected, int32_t newval)
{
    return __sync_bool_compare_and_swap(ptr, expected, newval);
}

/* Удаление завершающего '\n' */
static void rtrim_nl(char *s)
{
    size_t l = strlen(s);
    if (l && s[l-1] == '\n') s[l-1] = '\0';
}

/* Строка времени для логов */
static void now_str(char *buf, size_t bufsz)
{
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, bufsz, "%Y-%m-%d %H:%M:%S", &tm);
}

/* Чистый выход при сигнале */
static void cleanup_and_exit(int signo)
{
    (void)signo;
    if (g_shm) { shmdt(g_shm); g_shm = NULL; }
    fprintf(stderr, "Exiting...\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int chat_id = -1, term_id = -1;

    if (argc >= 2) chat_id = atoi(argv[1]);
    if (argc >= 3) term_id = atoi(argv[2]);

    if (chat_id < 0) {
        printf("Enter chat id (integer): ");
        if (scanf("%d", &chat_id) != 1) { fprintf(stderr, "Invalid chat id\n"); return 1; }
    }
    if (term_id < 0) {
        printf("Enter terminal id (integer): ");
        if (scanf("%d", &term_id) != 1) { fprintf(stderr, "Invalid term id\n"); return 1; }
    }

    g_key = (key_t)(K_SHM_KEY_BASE + (key_t)chat_id);
    printf("SHM-Chat: chat_id=%d term_id=%d\n", chat_id, term_id);
    printf("Using shm key = 0x%08x\n", (unsigned)g_key);

    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    g_shmid = shmget(g_key, (size_t)K_PAGE_SIZE, IPC_CREAT | K_FULL_ACCESS);
    if (g_shmid < 0) { perror("shmget"); return 1; }

    void *p = shmat(g_shmid, NULL, 0);
    if (p == (void*)-1) { perror("shmat"); return 1; }
    g_shm = (struct shm_area *)p;

    /* Инициализация при первом создании (защищённо) */
    if (g_shm->state != 0 && g_shm->state != 1) {
        memset(g_shm, 0, K_PAGE_SIZE);
        g_shm->state = 0;
        g_shm->msg_seq = 0;
    }

    char my_tty[MAX_TTY_LEN];
    char *tn = ttyname(fileno(stdin));
    if (tn) {
        strncpy(my_tty, tn, sizeof(my_tty)-1);
        my_tty[sizeof(my_tty)-1] = '\0';
    } else {
        strncpy(my_tty, "<unknown>", sizeof(my_tty)-1);
        my_tty[sizeof(my_tty)-1] = '\0';
    }

    printf("This process tty: %s\n", my_tty);
    printf("Type messages (empty line to quit).\n");

    /* Убираем остаток после scanf */
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); shmdt(g_shm); return 1; }

    if (pid == 0) {
        /* CHILD: слушаем новые сообщения, отслеживаем локальный last_seen_seq */
        int32_t last_seen_seq = g_shm->msg_seq; /* при старте не показываем старые сообщения */
        for (;;) {
            int32_t cur_seq = g_shm->msg_seq;
            if (cur_seq > last_seen_seq) {
                /* новое сообщение появилось */
                if (g_shm->sender_tid != term_id) {
                    char ts[32];
                    now_str(ts, sizeof(ts));
                    printf("[%s] From term %d (%s): %s\n", ts,
                           g_shm->sender_tid, g_shm->sender_tty, g_shm->msg);
                    fflush(stdout);
                }
                last_seen_seq = cur_seq;
            }
            usleep(K_DELAY_US);
        }
    } else {
        /* PARENT: вводим и отправляем */
        char inbuf[1024];
        for (;;) {
            if (!fgets(inbuf, sizeof(inbuf), stdin)) break;
            rtrim_nl(inbuf);
            if (inbuf[0] == '\0') break;

            char outmsg[MAX_MSG_LEN];
            int n = snprintf(outmsg, sizeof(outmsg), "term%d: %s", term_id, inbuf);
            if (n < 0) { fprintf(stderr, "Formatting error\n"); continue; }
            if ((size_t)n >= sizeof(outmsg)) {
                fprintf(stderr, "Warning: message truncated to %zu bytes\n", sizeof(outmsg)-1);
                outmsg[sizeof(outmsg)-1] = '\0';
            }

            /* Захват lock: ждём пока state == 0, затем ставим 1. */
            int tries = 0;
            while (!cas_int32(&g_shm->state, 0, 1)) {
                usleep(K_DELAY_US);
                if (++tries > 1000) { fprintf(stderr, "Busy, retrying...\n"); tries = 0; }
            }

            /* Пишем поля (под lock) */
            g_shm->sender_tid = term_id;
            strncpy(g_shm->sender_tty, my_tty, sizeof(g_shm->sender_tty)-1);
            g_shm->sender_tty[sizeof(g_shm->sender_tty)-1] = '\0';
            strncpy(g_shm->msg, outmsg, sizeof(g_shm->msg)-1);
            g_shm->msg[sizeof(g_shm->msg)-1] = '\0';

            /* Увеличиваем seq — это пометка "новое сообщение" */
            g_shm->msg_seq = g_shm->msg_seq + 1;

            /* Снимаем lock (state = 0) — читатели увидят новое msg_seq */
            g_shm->state = 0;

            usleep(K_DELAY_US);
        }

        shmdt(g_shm);
        g_shm = NULL;
    }

    return 0;
}