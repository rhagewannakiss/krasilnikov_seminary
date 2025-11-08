#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* cmp должен возвращать 0, если "равны" по выбранному критерию (как strcmp). */

/* сравнение по количеству букв: возвращает 0 если equal, !=0 иначе */
int count_cmp(const char *a, const char *b)
{
    int ca = 0, cb = 0;
    while (*a) { if (isalpha((unsigned char)*a)) ca++; a++; }
    while (*b) { if (isalpha((unsigned char)*b)) cb++; b++; }
    return (ca == cb) ? 0 : (ca < cb ? -1 : 1);
}

/* классическое strcmp (оборачиваем, чтобы сигнатуры совпали) */
int strcmp_wrap(const char *a, const char *b)
{
    return strcmp(a, b);
}

void check(const char *a, const char *b, int (*cmp)(const char *, const char *))
{
    printf("Проверка на совпадение.\n");
    if (!(*cmp)(a, b)) printf("Равны\n");
    else printf("Не равны\n");
}

int main(int argc, char *argv[])
{
    char s1[80], s2[80];
    int (*p)(const char *, const char *);

    /* выбор функции по аргументу */
    if (argc >= 2 && strcmp(argv[1], "count") == 0) {
        p = count_cmp;
        printf("Используется сравнение по количеству букв (count_cmp).\n");
    } else {
        p = strcmp_wrap;
        printf("Используется strcmp (лексикографическое сравнение).\n");
    }

    printf("Введите первую строку (max 79 символов):\n");
    if (!fgets(s1, sizeof(s1), stdin)) {
        fprintf(stderr, "Ошибка ввода\n");
        return 1;
    }
    /* удаляем возможный '\n' */
    s1[strcspn(s1, "\n")] = '\0';

    printf("Введите вторую строку (max 79 символов):\n");
    if (!fgets(s2, sizeof(s2), stdin)) {
        fprintf(stderr, "Ошибка ввода\n");
        return 1;
    }
    s2[strcspn(s2, "\n")] = '\0';

    check(s1, s2, p);

    return 0;
}
