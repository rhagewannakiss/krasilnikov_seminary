#include "task_1.h"

#include <assert.h>
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>

static void ensure_capacity(Buffer* buffer, int need );
static int  is_english_letter(const wchar_t c);
static int  is_russian_letter(const wchar_t c);
static void print_word_stats(const wchar_t* start, int len);


Buffer* BuffCtor(Buffer* buffer) {
    assert(buffer != NULL);

    buffer->capacity = default_capacity + 1;
    buffer->word_cnt = 0;

    buffer->data = (wchar_t*)calloc(buffer->capacity, sizeof(wchar_t));
    assert(buffer->data != NULL);

    return buffer;
}

void BuffDtor(Buffer* buffer) {
    assert(buffer != NULL);

    free(buffer->data);
    buffer->data     = NULL;
    buffer->capacity  = 0;
    buffer->word_cnt = 0;
}


int read_line_to_buffer(Buffer* buffer) {
    assert(buffer != NULL);

    int    cur = 0;
    wint_t wc  = 0;

    while ((wc = getwchar()) != WEOF && wc != L'\n') {
        ensure_capacity(buffer, cur + 1);
        buffer->data[cur++] = (wchar_t)wc;
    }
    ensure_capacity(buffer, cur);

    buffer->data[cur] = L'\0';

    return cur;
}

void process_and_print_words(Buffer* buffer, int len) {
    assert(buffer != NULL
        && buffer->data != NULL);

    int i = 0;

    while (i < len) {
        while (i < len && iswspace(buffer->data[i])) {
                i++;
            }

        if (i >= len) break;

        int start = i;
        while (i < len && !iswspace(buffer->data[i])) {
            i++;
        }
        int word_len = i - start;

        print_word_stats(buffer->data + start, word_len);
        buffer->word_cnt++;
    }

    wprintf(L"number of words: %d\n", buffer->word_cnt);
}



static int is_english_letter(const wchar_t c) {
    return (c >= L'A' && c <= L'Z')
        || (c >= L'a' && c <= L'z');
}

static int is_russian_letter(const wchar_t c) {
    return (c >= 0x0410 && c <= 0x044F)
         || c == 0x0401
         || c == 0x0451;
}


static void print_word_stats(const wchar_t* start, int len) {
    assert(start != NULL);

    if (len == 0) return;

    int digits = 0;
    int eng    = 0;
    int rus    = 0;

    for (int k = 0; k < len; k++) {
        wchar_t c = start[k];
        if (iswdigit(c)) {
            digits++;
        } else if (is_english_letter(c)) {
            eng++;
        } else if (is_russian_letter(c)) {
            rus++;
        }
    }

    wprintf(L"word: '%.*ls'  digits: %d  eng letters: %d  rus letters: %d\n",
            (int)len, start,   digits,           eng,                rus);
}

static void ensure_capacity(Buffer* buffer, int need) {
    assert(buffer != NULL);
    if (need + 1 <= buffer->capacity) {
        return;
    }

    int new_cap = buffer->capacity ? buffer->capacity : default_capacity;

    while (new_cap < need + 1) {
        int prev = new_cap;
        new_cap  = new_cap * scale_factor;
        if (new_cap <= prev) {
            new_cap = prev + 1;
        }
    }

    wchar_t* tmp = (wchar_t*)realloc(buffer->data, (new_cap + 1) * sizeof(wchar_t));
    if (!tmp) {
        fwprintf(stderr, L"Reallocation error in ensure_capacity func!\n");
        free(buffer->data);
        abort();
    }

    if (new_cap > buffer->capacity) {
        memset(tmp + buffer->capacity, 0, (new_cap + 1 - buffer->capacity) * sizeof(wchar_t));
    }
    buffer->data = tmp;
    buffer->capacity = new_cap;
}
