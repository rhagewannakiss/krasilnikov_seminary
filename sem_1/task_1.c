#include "task_1.h"

#include <assert.h>
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>

//на вход приходи строка а потом печатаем строку с числом слов, потом вывод слова, сколько в нем цифр, английских букв и русских букв
//пример: ввод:"лод7df ыщао98sfc 34kfлд" вывод: "слово: лод7df цифр: 1 английских букв: 2 русских букв: 3 .......и тд"

static const int default_capacity = 4096; //page
static const int scale_factor     = 2;

typedef struct Buffer {
    int      max_cap;
    wchar_t* data;
    int      word_cnt;
} Buffer;

static Buffer* BuffCtor(Buffer* buffer);
static void    BuffDtor(Buffer* buffer);
static void    realloc_if_needed(Buffer* buffer);

int main() {
    setlocale(LC_ALL, "");

    Buffer buffer = {};
    BuffCtor(&buffer);

    wint_t ch       = 0;
    size_t cur_char = 0;

    wprintf(L"Enter your string: \n");
    while ((ch = getwchar()) != WEOF
         && ch != L'\n') {
            if (cur_char + 1 >= buffer.max_cap) {
                realloc_if_needed(&buffer);
            }
        buffer.data[cur_char] = (wchar_t)ch;
        cur_char++;
    }
    buffer.data[cur_char] = L'\0';

    if(cur_char == 0) {
        wprintf(L"Empty string!\n");
        BuffDtor(&buffer);
        return 0;
    }

    int i = 0;

    while (i < cur_char) {
        while (iswspace(buffer.data[i])) {
            i++;
        }

        if (i >= cur_char) break;

        size_t start = i;
        int digits   = 0;
        int eng      = 0;
        int rus      = 0;

        while (i < cur_char
            && !iswspace(buffer.data[i])) {
            wchar_t c = buffer.data[i];

        if (iswdigit(c)) {
                        digits++;
                    } else if ((c >= L'A'
                             && c <= L'Z')
                             || (c >= L'a'
                                && c <= L'z')) {
                        eng++;
                    } else if ((c >= L'\u0410'
                             && c <= L'\u044F')
                             || c == L'\u0401'
                             || c == L'\u0451') {
                        rus++;
                    }
                    i++;
                }

                wprintf(L"word: '%.*ls'  number of digits: %d  eng letters: %d  rus letters: %d\n",
                    (int)(i - start), buffer.data + start, digits, eng, rus);

                buffer.word_cnt++;
            }

    wprintf(L"number of words: %d\n", buffer.word_cnt);

    BuffDtor(&buffer);
    return 0;
}



//-------------buffer------------
static Buffer* BuffCtor(Buffer* buffer) {
    assert(buffer != NULL);

    buffer->data = (wchar_t*)calloc(default_capacity + 1, sizeof(wchar_t));
    assert(buffer->data != NULL);

    buffer->max_cap = default_capacity;
    buffer->word_cnt = 0;
    return buffer;
}

static void BuffDtor(Buffer* buffer) {
    assert(buffer != NULL);

    free(buffer->data);
    buffer->data     = NULL;
    buffer->max_cap  = 0;
    buffer->word_cnt = 0;
}

static void realloc_if_needed(Buffer* buffer) {
    assert(buffer != NULL);

    buffer->max_cap = default_capacity * scale_factor;
    buffer->data = (wchar_t*)realloc(buffer->data, (size_t)(buffer->max_cap + 1) * sizeof(wchar_t));
    assert(buffer->data);
}
