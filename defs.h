#ifndef DEFS_H
#define DEFS_H

#define MAT_SIZE 8192UL
#define BLOCK_SIZE 512
#define BLOCK_COUNT 16

typedef unsigned long long size_type;
typedef int value_type;
typedef value_type(Vector)[MAT_SIZE];
typedef Vector(Mat)[MAT_SIZE];

#endif // DEFS_H