#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PI 3.1415926535
#define QQ 40 // Quality of Quarter Rings
#define QC 40 // Quality of Circle
#define SM 0.9 // Size Multiplier to avoid false-positive collisions

static bool isValidWire(const char *w);
static void lbatchLine(double x, double y, double a, double l, double t, size_t *ni, size_t *nv, size_t *i, double *v);
static void lbatchRingSlice(double x, double y, double r, double t, double a, double o, size_t *ni, size_t *nv, size_t *i, double *v);

int main(int argc, char **argv) {
    if (argc != 2 || strspn(argv[1], "RUD") != strlen(argv[1])) return 42;
    return !isValidWire(argv[argc - 1]);
}

static bool isValidWire(const char *w) {
    size_t l = strlen(w);
    size_t ni = 0;
    size_t nv = 0;

    for (size_t i = 0; i < l; ++i) {
        ni += w[i] == 'R' ? 8 : QQ * 4;
        nv += w[i] == 'R' ? 4 : QQ * 2;
    }

    size_t *i = malloc(ni * sizeof(*i));
    double *v = malloc(2 * nv * sizeof(*v));

    double x = 0;
    double y = 0;
    char dir = 'R';
    ni = nv = 0;
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

    // TODO: collision detection

    return true;
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
