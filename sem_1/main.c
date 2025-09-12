#include "task_1.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <string.h>

//на вход приходи строка а потом печатаем строку с числом слов, потом вывод слова, сколько в нем цифр, английских букв и русских букв
//пример: ввод:"лод7df ыщао98sfc 34kfлд" вывод: "слово: лод7df цифр: 1 английских букв: 2 русских букв: 3 .......и тд"


int main(void) {
    setlocale(LC_ALL, "");

    Buffer buffer = {};
    BuffCtor(&buffer);

    wprintf(L"Enter the string:\n");
    int len = read_line_to_buffer(&buffer);

    if (len == 0) {
        wprintf(L"Empty string!\n");
        BuffDtor(&buffer);
        return 0;
    }

    process_and_print_words(&buffer, len);

    BuffDtor(&buffer);
    return 0;
}
