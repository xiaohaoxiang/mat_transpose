#ifndef TRANSPOSE_H
#define TRANSPOSE_H

#include "defs.h"
#include <pthread.h>

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef enum
{

    alldone = 0,
    transblk = 1,
    swapblk = 2
} TransBlockTaskType;

typedef struct
{
    TransBlockTaskType tp;
    Rect rect;
} TransBlockTask;

typedef struct
{
    Vector *M;
    int start;
    int end;
} terraceData;

void trans_serial(Mat M);

void trans_block(Mat M, const int HC);
void trans_block_thread(Mat M);
void push_task(TransBlockTask *tsk);
void get_task(TransBlockTask *tsk);
void trans_block_transrect(Mat M, Rect *rect);
void trans_block_swaprect(Mat M, Rect *rect, void *buffer);

void trans_terrace(Mat M, const int HC);
void trans_terrace_thread(terraceData *data);
void trans_terrace_range(Mat M, int start, int end);

void swap(value_type *x, value_type *y);

#endif // TRANSPOSE_H