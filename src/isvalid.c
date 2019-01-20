#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "main.h"

#define SM 0.9 // Size Multiplier

static void lbatch(const char *w, size_t *i, double *v);
static void lbatchLine(double x, double y, double a, double l, double t, size_t *ni, size_t *nv, size_t *i, double *v);
static void lbatchRingSlice(double x, double y, double r, double t, double a, double o, size_t *ni, size_t *nv, size_t *i, double *v);
static void lbatchCircle(double x, double y, double r, size_t *ni, size_t *nv, size_t *i, double *v);
static bool segmentsCollide(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);
static void rayCollision(double ax, double ay, double aa, double bx, double by, double ba, double *al, double *bl);

bool isValidWire(const char *w) {
    size_t l = strlen(w);
    size_t ni = 8 + QC * 4;
    size_t nv = 4 + QC * 2;

    for (size_t i = 0; i < l; ++i) {
        ni += w[i] == 'R' ? 8 : QQ * 4;
        nv += w[i] == 'R' ? 4 : QQ * 2;
    }

    size_t *i = malloc(ni * sizeof(*i));
    double *v = malloc(2 * nv * sizeof(*v));

    lbatch(w, i, v);

    size_t ioffset = 0;
    for (size_t j = l - 1; j < l; --j) {
        size_t prevIoffset = ioffset;
        ioffset += w[j] == 'R' ? 8 : QQ * 4;
        for (size_t g = ioffset; g < ni; g += 2) {
            for (size_t h = prevIoffset; h < ioffset; h += 2) {
                double x0 = v[i[h + 0] * 2 + 0];
                double y0 = v[i[h + 0] * 2 + 1];
                double x1 = v[i[h + 1] * 2 + 0];
                double y1 = v[i[h + 1] * 2 + 1];
                double x2 = v[i[g + 0] * 2 + 0];
                double y2 = v[i[g + 0] * 2 + 1];
                double x3 = v[i[g + 1] * 2 + 0];
                double y3 = v[i[g + 1] * 2 + 1];
                if (segmentsCollide(x0, y0, x1, y1, x2, y2, x3, y3)) {
                    return false;
                }
            }
        }
    }

    return true;
}

static void lbatch(const char *w, size_t *i, double *v) {
    size_t l = strlen(w);
    double x = 0;
    double y = 0;
    size_t ni = 0;
    size_t nv = 0;
    char dir = 'R';

    for (size_t j = l - 1; j < l; --j) {
        if (dir == 'U') {
            if (w[j] == 'U') {
                lbatchRingSlice(x - 1, y, 1, SM, 0, SM * PI / 2, &ni, &nv, i, v);
                x -= 1;
                y += 1;
                dir = 'L';
            } else if (w[j] == 'D') {
                lbatchRingSlice(x + 1, y, 1, SM, PI, SM * -PI / 2, &ni, &nv, i, v);
                x += 1;
                y += 1;
                dir = 'R';
            } else {
                lbatchLine(x, y, PI / 2, SM * PI / 2, SM, &ni, &nv, i, v);
                y += PI / 2;
            }
        } else if (dir == 'D') {
            if (w[j] == 'U') {
                lbatchRingSlice(x + 1, y, 1, SM, PI, SM * PI / 2, &ni, &nv, i, v);
                x += 1;
                y -= 1;
                dir = 'R';
            } else if (w[j] == 'D') {
                lbatchRingSlice(x - 1, y, 1, SM, 0, SM * -PI / 2, &ni, &nv, i, v);
                x -= 1;
                y -= 1;
                dir = 'L';
            } else {
                lbatchLine(x, y, -PI / 2, SM * PI / 2, SM, &ni, &nv, i, v);
                y -= PI / 2;
            }
        } else if (dir == 'L') {
            if (w[j] == 'U') {
                lbatchRingSlice(x, y - 1, 1, SM, PI / 2, SM * PI / 2, &ni, &nv, i, v);
                x -= 1;
                y -= 1;
                dir = 'D';
            } else if (w[j] == 'D') {
                lbatchRingSlice(x, y + 1, 1, SM, -PI / 2, SM * -PI / 2, &ni, &nv, i, v);
                x -= 1;
                y += 1;
                dir = 'U';
            } else {
                lbatchLine(x, y, 0, SM * -PI / 2, SM, &ni, &nv, i, v);
                x -= PI / 2;
            }
        } else {
            if (w[j] == 'U') {
                lbatchRingSlice(x, y + 1, 1, SM, -PI / 2, SM * PI / 2, &ni, &nv, i, v);
                x += 1;
                y += 1;
                dir = 'U';
            } else if (w[j] == 'D') {
                lbatchRingSlice(x, y - 1, 1, SM, PI / 2, SM * -PI / 2, &ni, &nv, i, v);
                x += 1;
                y -= 1;
                dir = 'D';
            } else {
                lbatchLine(x, y, 0, SM * PI / 2, SM, &ni, &nv, i, v);
                x += PI / 2;
            }
        }
    }

    lbatchLine(-99, 0, 0, 99 - 1 + SM, SM, &ni, &nv, i, v);
    lbatchCircle(0,  1, 0.5 * SM, &ni, &nv, i, v);
    lbatchCircle(0, -1, 0.5 * SM, &ni, &nv, i, v);
}

static void lbatchLine(double x, double y, double a, double l, double t, size_t *ni, size_t *nv, size_t *i, double *v) {
    i[*ni + 0] = *nv + 0;
    i[*ni + 1] = *nv + 1;
    i[*ni + 2] = *nv + 1;
    i[*ni + 3] = *nv + 2;
    i[*ni + 4] = *nv + 2;
    i[*ni + 5] = *nv + 3;
    i[*ni + 6] = *nv + 3;
    i[*ni + 7] = *nv + 0;

    double dx = sin(a) * t / 2;
    double dy = cos(a) * t / 2;
    double X = x + cos(a) * l;
    double Y = y + sin(a) * l;
    v[*nv * 2 + 0] = x - dx;
    v[*nv * 2 + 1] = y + dy;
    v[*nv * 2 + 2] = x + dx;
    v[*nv * 2 + 3] = y - dy;
    v[*nv * 2 + 4] = X + dx;
    v[*nv * 2 + 5] = Y - dy;
    v[*nv * 2 + 6] = X - dx;
    v[*nv * 2 + 7] = Y + dy;

    *ni += 8;
    *nv += 4;
}

static void lbatchRingSlice(double x, double y, double r, double t, double o, double a, size_t *ni, size_t *nv, size_t *i, double *v) {
    for (size_t j = 0; j < QQ - 1; ++j) {
        i[*ni + j * 4 + 0] = *nv + j * 2 + 0;
        i[*ni + j * 4 + 1] = *nv + j * 2 + 2;
        i[*ni + j * 4 + 2] = *nv + j * 2 + 1;
        i[*ni + j * 4 + 3] = *nv + j * 2 + 3;
    }
    i[*ni + QQ * 4 - 4] = *nv + 0;
    i[*ni + QQ * 4 - 3] = *nv + 1;
    i[*ni + QQ * 4 - 2] = *nv + QQ * 2 - 2;
    i[*ni + QQ * 4 - 1] = *nv + QQ * 2 - 1;

    double da = a / (QQ - 1);
    for (size_t j = 0; j < QQ; ++j) {
        double angle = o + da * j;
        double s = sin(angle);
        double c = cos(angle);
        v[*nv * 2 + j * 4 + 0] = x + c * (r - t / 2);
        v[*nv * 2 + j * 4 + 1] = y + s * (r - t / 2);
        v[*nv * 2 + j * 4 + 2] = x + c * (r + t / 2);
        v[*nv * 2 + j * 4 + 3] = y + s * (r + t / 2);
    }

    *ni += QQ * 4;
    *nv += QQ * 2;
}

static void lbatchCircle(double x, double y, double r, size_t *ni, size_t *nv, size_t *i, double *v) {
    for (size_t j = 0; j < QC - 1; ++j) {
        i[*ni + j * 2 + 0] = *nv + j + 0;
        i[*ni + j * 2 + 1] = *nv + j + 1;
    }
    i[*ni + QC * 2 - 2] = *nv + QC - 1;
    i[*ni + QC * 2 - 1] = *nv;

    double da = PI * 2 / QC;
    for (size_t j = 0; j < QC; ++j) {
        double angle = da * j;
        double s = sin(angle);
        double c = cos(angle);
        v[*nv * 2 + j * 2 + 0] = x + c * r;
        v[*nv * 2 + j * 2 + 1] = y + s * r;
    }

    *ni += QC * 2;
    *nv += QC;
}

static bool segmentsCollide(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3) {
    double aa = atan2(y1 - y0, x1 - x0);
    double al = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
    double ba = atan2(y3 - y2, x3 - x2);
    double bl = sqrt((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));
    double aL, bL;
    rayCollision(x0, y0, aa, x2, y2, ba, &aL, &bL);
    return !(isnan(aL) || isnan(bL) || aL < 0 || bL < 0 || aL > al || bL > bl);
}

static void rayCollision(double ax, double ay, double aa, double bx, double by, double ba, double *al, double *bl) {
    double ac = cosf(aa);
    double bc = cosf(ba);
    double as = sinf(aa);
    double bs = sinf(ba);
    double dx = bx - ax;
    double dy = by - ay;
    double det = bc * as - ac * bs;
    if (det == 0) {
        *al = *bl = NAN;
    } else {
        *al = (dy * bc - dx * bs) / det;
        *bl = (dy * ac - dx * as) / det;
    }
}
