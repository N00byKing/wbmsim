#include "lib.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979

Batch batchNew(void) {
    return (Batch){0, 0, 0, 0, NULL, NULL};
}

void batchDel(Batch *b) {
    free(b->i);
    free(b->v);
    memset(b, 0, sizeof(*b));
}

void batchClear(Batch *b) {
    b->ni = b->nv = 0;
}

void batchDraw(const Batch *b) {
    xTris(b->ni, b->i, b->v);
}

void batchAny(Batch*b,size_t ni,const uint32_t*i,size_t nv,const XVertex*v) {
    if (b->mi < b->ni + ni) {
        if (b->mi == 0) {
            b->mi = 1;
        }
        while (b->mi < b->ni + ni) {
            b->mi *= 2;
        }
        b->i = realloc(b->i, b->mi * sizeof(*b->i));
    }
    if (i != NULL) {
        memcpy(b->i + b->ni, i, ni * sizeof(*i));
        b->ni += ni;
    }

    if (b->mv < b->nv + nv) {
        if (b->mv == 0) {
            b->mv = 1;
        }
        while (b->mv < b->nv + nv) {
            b->mv *= 2;
        }
        b->v = realloc(b->v, b->mv * sizeof(*b->v));
    }
    if (v != NULL) {
        memcpy(b->v + b->nv, v, nv * sizeof(*v));
        b->nv += nv;
    }
}

void batchRect(Batch *b, const float *xywh, const uint8_t *rgb) {
    XVertex v0 = {xywh[0],           xywh[1],           rgb[0], rgb[1], rgb[2]};
    XVertex v1 = {xywh[0] + xywh[2], xywh[1],           rgb[0], rgb[1], rgb[2]};
    XVertex v2 = {xywh[0] + xywh[2], xywh[1] + xywh[3], rgb[0], rgb[1], rgb[2]};
    XVertex v3 = {xywh[0],           xywh[1] + xywh[3], rgb[0], rgb[1], rgb[2]};
    uint32_t i[] = {b->nv+0,b->nv+1,b->nv+2,b->nv+2,b->nv+3,b->nv+0};
    batchAny(b, 6, i, 4, (XVertex[]){v0, v1, v2, v3});
}

void batchRectLine(Batch*b,const float*xywh,float ti,float to,const uint8_t*rgb){
    batchRect(b, (float[]){xywh[0] - to,       xywh[1],            ti + to, xywh[3]}, rgb);
    batchRect(b, (float[]){xywh[0],            xywh[1] - to,       xywh[2], ti + to}, rgb);
    batchRect(b, (float[]){xywh[0],            xywh[1]+xywh[3]-ti, xywh[2], ti + to}, rgb);
    batchRect(b, (float[]){xywh[0]+xywh[2]-ti, xywh[1],            ti + to, xywh[3]}, rgb);
}

void batchLine(Batch*b,float x,float y,float a,float l,float t,const uint8_t*rgb){
    float dx = sinf(a) * t / 2;
    float dy = cosf(a) * t / 2;
    float X = x + cosf(a) * l;
    float Y = y + sinf(a) * l;
    uint32_t i[] = {b->nv+0,b->nv+1,b->nv+2,b->nv+2,b->nv+3,b->nv+0};
    XVertex v0 = {x - dx, y + dy, rgb[0], rgb[1], rgb[2]};
    XVertex v1 = {x + dx, y - dy, rgb[0], rgb[1], rgb[2]};
    XVertex v2 = {X + dx, Y - dy, rgb[0], rgb[1], rgb[2]};
    XVertex v3 = {X - dx, Y + dy, rgb[0], rgb[1], rgb[2]};
    batchAny(b, 6, i, 4, (XVertex[]){v0, v1, v2, v3});
}

void batchCircle(Batch*b,float x,float y,float r,size_t n,const uint8_t*rgb) {
    uint32_t *i = malloc(n * 3 * sizeof(*i));
    XVertex *v = malloc((n + 1) * sizeof(*v));

    for (size_t j = 0; j < n - 1; ++j) {
        i[j * 3 + 0] = b->nv + 0;
        i[j * 3 + 1] = b->nv + j + 1;
        i[j * 3 + 2] = b->nv + j + 2;
    }
    i[n * 3 - 3] = b->nv + 0;
    i[n * 3 - 2] = b->nv + n;
    i[n * 3 - 1] = b->nv + 1;

    float da = PI * 2 / n;
    v[0] = (XVertex){x, y, rgb[0], rgb[1], rgb[2]};
    for (size_t j = 0; j < n; ++j) {
        v[j + 1] = (XVertex){x + cosf(da * j) * r, y + sinf(da * j) * r, rgb[0], rgb[1], rgb[2]};
    }

    batchAny(b, n * 3, i, n + 1, v);
}

void batchPieSlice(Batch*b,float x,float y,float r,float o,float a,size_t n,const uint8_t*rgb){
    uint32_t *i = malloc((n - 1) * 3 * sizeof(*i));
    XVertex *v = malloc((n + 1) * sizeof(*v));

    for (size_t j = 0; j < n - 1; ++j) {
        i[j * 3 + 0] = b->nv + 0;
        i[j * 3 + 1] = b->nv + j + 1;
        i[j * 3 + 2] = b->nv + j + 2;
    }

    float da = a / (n - 1);
    v[0] = (XVertex){x, y, rgb[0], rgb[1], rgb[2]};
    for (size_t j = 0; j < n; ++j) {
        v[j + 1] = (XVertex){x + cosf(da * j + o) * r, y + sinf(da * j + o) * r, rgb[0], rgb[1], rgb[2]};
    }

    batchAny(b, (n - 1) * 3, i, n + 1, v);
}

void batchRing(Batch*b,float x,float y,float r,float t,size_t n,const uint8_t*rgb){
    uint32_t *i = malloc(n * 6 * sizeof(*i));
    XVertex *v = malloc(n * 2 * sizeof(*v));

    for (size_t j = 0; j < n - 1; ++j) {
        i[j * 6 + 0] = b->nv + j * 2 + 0;
        i[j * 6 + 1] = b->nv + j * 2 + 1;
        i[j * 6 + 2] = b->nv + j * 2 + 3;
        i[j * 6 + 3] = b->nv + j * 2 + 3;
        i[j * 6 + 4] = b->nv + j * 2 + 2;
        i[j * 6 + 5] = b->nv + j * 2 + 0;
    }
    i[n * 6 - 6] = b->nv + n * 2 - 2;
    i[n * 6 - 5] = b->nv + n * 2 - 1;
    i[n * 6 - 4] = b->nv + 1;
    i[n * 6 - 3] = b->nv + 1;
    i[n * 6 - 2] = b->nv + 2;
    i[n * 6 - 1] = b->nv + n * 2 - 2;

    float da = PI * 2 / n;
    float r0 = r - t / 2;
    float r1 = r + t / 2;
    for (size_t j = 0; j < n; ++j) {
        v[j * 2 + 0] = (XVertex){x + cosf(da * j) * r0, y + sinf(da * j) * r0, rgb[0], rgb[1], rgb[2]};
        v[j * 2 + 1] = (XVertex){x + cosf(da * j) * r1, y + sinf(da * j) * r1, rgb[0], rgb[1], rgb[2]};
    }

    batchAny(b, n * 6, i, n * 2, v);
}

void batchRingSlice(Batch*b,float x,float y,float r,float t,float o,float a,size_t n,const uint8_t*rgb){
    uint32_t *i = malloc((n - 1) * 6 * sizeof(*i));
    XVertex *v = malloc(n * 2 * sizeof(*v));

    for (size_t j = 0; j < n - 1; ++j) {
        i[j * 6 + 0] = b->nv + j * 2 + 0;
        i[j * 6 + 1] = b->nv + j * 2 + 1;
        i[j * 6 + 2] = b->nv + j * 2 + 3;
        i[j * 6 + 3] = b->nv + j * 2 + 3;
        i[j * 6 + 4] = b->nv + j * 2 + 2;
        i[j * 6 + 5] = b->nv + j * 2 + 0;
    }

    float da = a / (n - 1);
    float r0 = r - t / 2;
    float r1 = r + t / 2;
    for (size_t j = 0; j < n; ++j) {
        v[j * 2 + 0] = (XVertex){x + cosf(da * j + o) * r0, y + sinf(da * j + o) * r0, rgb[0], rgb[1], rgb[2]};
        v[j * 2 + 1] = (XVertex){x + cosf(da * j + o) * r1, y + sinf(da * j + o) * r1, rgb[0], rgb[1], rgb[2]};
    }

    batchAny(b, (n - 1) * 6, i, n * 2, v);
}

