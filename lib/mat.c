#include "lib.h"

#include <math.h>

void matScl(float *m, float x, float y) {
    m[0] = x; m[3] = 0; m[6] = 0;
    m[1] = 0; m[4] = y; m[7] = 0;
    m[2] = 0; m[5] = 0; m[8] = 1;
}

void matTrans(float *m, float x, float y) {
    m[0] = 1; m[3] = 0; m[6] = x;
    m[1] = 0; m[4] = 1; m[7] = y;
    m[2] = 0; m[5] = 0; m[8] = 1;
}

void matRot(float *m, float a) {
    float s = sinf(a);
    float c = cosf(a);
    m[0] = c; m[3] =-s; m[6] = 0;
    m[1] = s; m[4] = c; m[7] = 0;
    m[2] = 0; m[5] = 0; m[8] = 1;
}

void matMul(float *ab, const float *a, const float *b) {
    ab[0] = a[0] * b[0] + a[3] * b[1] + a[6] * b[2];
    ab[3] = a[0] * b[3] + a[3] * b[4] + a[6] * b[5];
    ab[6] = a[0] * b[6] + a[3] * b[7] + a[6] * b[8];

    ab[1] = a[1] * b[0] + a[4] * b[1] + a[7] * b[2];
    ab[4] = a[1] * b[3] + a[4] * b[4] + a[7] * b[5];
    ab[7] = a[1] * b[6] + a[4] * b[7] + a[7] * b[8];

    ab[2] = a[2] * b[0] + a[5] * b[1] + a[8] * b[2];
    ab[5] = a[2] * b[3] + a[5] * b[4] + a[8] * b[5];
    ab[8] = a[2] * b[6] + a[5] * b[7] + a[8] * b[8];
}

void matMulVec(float *mv, const float *m, const float *v) {
    mv[0] = m[0] * v[0] + m[3] * v[1] + m[6] * v[2];
    mv[1] = m[1] * v[0] + m[4] * v[1] + m[7] * v[2];
    mv[2] = m[2] * v[0] + m[5] * v[1] + m[8] * v[2];
}
