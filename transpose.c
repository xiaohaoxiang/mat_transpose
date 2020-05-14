#include "transpose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void trans_serial(Mat M)
{
    for (int i = 1; i < MAT_SIZE; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            swap(M[i] + j, M[j] + i);
        }
    }
}

static pthread_mutex_t transBlockStateMtx = PTHREAD_MUTEX_INITIALIZER;
static int transBlockState[BLOCK_COUNT][BLOCK_COUNT];

static TransBlockTask transBlockTaskList[BLOCK_COUNT * BLOCK_COUNT * 2];
static int transBlockTaskCount;
static pthread_mutex_t transBlockTaskListMtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t transBlockTaskListCond = PTHREAD_COND_INITIALIZER;
static int transBlockTotalTaskCount = BLOCK_COUNT * BLOCK_COUNT + BLOCK_COUNT * (BLOCK_COUNT - 1) / 2;

void trans_block(Mat M, const int HC)
{
    pthread_mutex_init(&transBlockStateMtx, NULL);
    pthread_mutex_init(&transBlockTaskListMtx, NULL);
    pthread_cond_init(&transBlockTaskListCond, NULL);
    transBlockTaskCount = 0;
    memset(transBlockState, 0, sizeof(transBlockState));
    transBlockTotalTaskCount = BLOCK_COUNT * BLOCK_COUNT + BLOCK_COUNT * (BLOCK_COUNT - 1) / 2;

    for (int i = 0; i < BLOCK_COUNT; ++i)
    {
        for (int j = 0; j < BLOCK_COUNT; ++j)
        {
            transBlockTaskList[transBlockTaskCount].tp = transblk;
            transBlockTaskList[transBlockTaskCount].rect.x = i * BLOCK_SIZE;
            transBlockTaskList[transBlockTaskCount].rect.y = j * BLOCK_SIZE;
            transBlockTaskList[transBlockTaskCount].rect.w = BLOCK_SIZE;
            transBlockTaskList[transBlockTaskCount].rect.h = BLOCK_SIZE;
            ++transBlockTaskCount;
        }
    }
    pthread_t *threads = (pthread_t *)malloc(HC * sizeof(pthread_t));
    for (int i = 0; i < HC; ++i)
    {
        pthread_create(threads + i, NULL, (void *(*)(void *))trans_block_thread, M);
    }
    for (int i = 0; i < HC; ++i)
    {
        pthread_join(threads[i], NULL);
    }
}

void trans_block_thread(Mat M)
{
    void *swapBuffer = malloc(BLOCK_SIZE * sizeof(value_type));
    for (TransBlockTask tsk;;)
    {
        get_task(&tsk);
        if (tsk.tp == alldone)
        {
            break;
        }
        switch (tsk.tp)
        {
        case transblk: {
            trans_block_transrect(M, &tsk.rect);
            int bx = tsk.rect.x / BLOCK_SIZE, by = tsk.rect.y / BLOCK_SIZE;
            if (bx != by)
            {
                pthread_mutex_lock(&transBlockStateMtx);
                if (transBlockState[by][bx])
                {
                    tsk.tp = swapblk;
                    push_task(&tsk);
                }
                else
                {
                    transBlockState[bx][by] = 1;
                }
                pthread_mutex_unlock(&transBlockStateMtx);
            }
            break;
        }
        case swapblk:
            trans_block_swaprect(M, &tsk.rect, swapBuffer);
            break;
        }
    }
    free(swapBuffer);
}

void push_task(TransBlockTask *tsk)
{
    pthread_mutex_lock(&transBlockTaskListMtx);
    transBlockTaskList[transBlockTaskCount++] = *tsk;
    pthread_mutex_unlock(&transBlockTaskListMtx);
    pthread_cond_signal(&transBlockTaskListCond);
}

void get_task(TransBlockTask *tsk)
{
    pthread_mutex_lock(&transBlockTaskListMtx);
    if (transBlockTotalTaskCount)
    {
        --transBlockTotalTaskCount;
        while (transBlockTaskCount <= 0)
        {
            pthread_cond_wait(&transBlockTaskListCond, &transBlockTaskListMtx);
        }
        *tsk = transBlockTaskList[--transBlockTaskCount];
    }
    else
    {
        tsk->tp = alldone;
    }
    pthread_mutex_unlock(&transBlockTaskListMtx);
}

void trans_block_transrect(Mat M, Rect *rect)
{
    for (int i = 1; i < rect->w; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            swap(M[rect->x + i] + rect->y + j, M[rect->x + j] + rect->y + i);
        }
    }
}

void trans_block_swaprect(Mat M, Rect *rect, void *buffer)
{
    for (int i = 0; i < rect->w; ++i)
    {
        memcpy(buffer, M[rect->x + i] + rect->y, rect->h * sizeof(value_type));
        memcpy(M[rect->x + i] + rect->y, M[rect->y + i] + rect->x, rect->h * sizeof(value_type));
        memcpy(M[rect->y + i] + rect->x, buffer, rect->h * sizeof(value_type));
    }
}

void trans_terrace(Mat M, const int HC)
{
    void *buffer = malloc(HC * (64 + sizeof(pthread_t)));
    pthread_t *threads = buffer + HC * 64;
    const int d = MAT_SIZE / (HC * 2);
    for (int i = 0; i < HC; ++i)
    {
        terraceData *curData = buffer + i * 64;
        curData->M = M;
        curData->start = i * d;
        curData->end = i + 1 == HC ? MAT_SIZE / 2 : (i + 1) * d;
        pthread_create(threads + i, NULL, (void *(*)(void *))trans_terrace_thread, curData);
    }
    for (int i = 0; i < HC; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    free(buffer);
}

void trans_terrace_thread(terraceData *data)
{
    trans_terrace_range(data->M, data->start, data->end);
    trans_terrace_range(data->M, MAT_SIZE - data->end, MAT_SIZE - data->start);
}

void trans_terrace_range(Mat M, int start, int end)
{
    for (int i = start; i < end; ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            swap(M[i] + j, M[j] + i);
        }
    }
}

inline void swap(value_type *x, value_type *y)
{
    value_type tmp = *x;
    *x = *y;
    *y = tmp;
}