#ifndef TASK_1_H_
#define TASK_1_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <string.h>

static const int default_capacity = 4096; //page
static const int scale_factor     = 2;

typedef struct Buffer {
    int      capacity;
    int      word_cnt;
    wchar_t* data;
} Buffer;


Buffer* BuffCtor(Buffer* buffer);
void    BuffDtor(Buffer* buffer);
int     read_line_to_buffer(Buffer* buffer);
void    process_and_print_words(Buffer* buffer, int len);

#endif //TASK_1_H_
