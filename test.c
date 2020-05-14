#include "test.h"
#include "cpuinfo.h"
#include "timer.h"
#include "transpose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    Mat *A = (Mat *)malloc(sizeof(Mat)), *B = (Mat *)malloc(sizeof(Mat));
    generate_mat(*A);

    const int repeat = 8;
    const unsigned HC = hardware_concurrency() * 2;

    unsigned long long sum = 0;
    printf("mode  threads  time/ms\n");
    for (int i = 0; i < repeat; ++i)
    {
        memcpy(B, A, sizeof(Mat));
        timepoint t0 = time_now();
        trans_serial(*B);
        sum += time_now() - t0;
    }
    printf("0     1        %.2f\n", sum / 10.0 / repeat);

    for (unsigned i = 1; i <= HC; ++i)
    {
        sum = 0;
        for (int j = 0; j < repeat; ++j)
        {
            memcpy(B, A, sizeof(Mat));
            timepoint t0 = time_now();
            trans_block(*B, i);
            sum += time_now() - t0;
        }
        printf("1     %-2d       %.2f\n", i, sum / 10.0 / repeat);
    }
    for (unsigned i = 1; i <= HC; ++i)
    {
        sum = 0;
        for (int j = 0; j < repeat; ++j)
        {
            memcpy(B, A, sizeof(Mat));
            timepoint t0 = time_now();
            trans_terrace(*B, i);
            sum += time_now() - t0;
        }
        printf("2     %-2d       %.2f\n", i, sum / 10.0 / repeat);
    }
    return 0;
}

int is_transposed(Mat A, Mat B)
{
    int cnt = 0;
    for (int i = 0; i < MAT_SIZE; ++i)
    {
        for (int j = 0; j < MAT_SIZE; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                ++cnt;
            }
        }
    }
    return cnt;
}

void generate_mat(Mat M)
{
    for (int i = 0; i < MAT_SIZE; ++i)
    {
        for (int j = 0; j < MAT_SIZE; ++j)
        {
            M[i][j] = rand();
        }
    }
}
