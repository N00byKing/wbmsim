#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define PI 3.1415926535
#define QQ 40 // Quality of Quarter Rings
#define SM 0.99 // Size Multiplier to avoid false-positive collisions

static bool isValidWire(const char *w);
static void lbatchLine(double x, double y, double a, double l, double t, size_t nv, size_t *i, double *v);
static void lbatchRingSlice(double x, double y, double r, double t, double a, double o, size_t nv, size_t *i, double *v);

int main(int argc, char **argv) {
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
                // TODO 1
            } else if (w[j] == 'D') {
                // TODO 2
            } else {
                // TODO 3
            }
        } else if (dir == 'D') {
            if (w[j] == 'U') {
                // TODO 4
            } else if (w[j] == 'D') {
                // TODO 5
            } else {
                // TODO 6
            }
        } else if (dir == 'L') {
            if (w[j] == 'U') {
                // TODO 7
            } else if (w[j] == 'D') {
                // TODO 8
            } else {
                // TODO 9
            }
        } else {
            if (w[j] == 'U') {
                // TODO 10
            } else if (w[j] == 'D') {
                // TODO 11
            } else {
                // TODO 12
            }
        }
    }

    return true;
}

static void lbatchLine(double x, double y, double a, double l, double t, size_t nv, size_t *i, double *v) {
    i[0] = nv + 0;
    i[1] = nv + 1;
    i[2] = nv + 1;
    i[3] = nv + 2;
    i[4] = nv + 2;
    i[5] = nv + 3;
    i[6] = nv + 3;
    i[7] = nv + 0;

    double dx = sin(a) * t / 2;
    double dy = cos(a) * t / 2;
    double X = x + cos(a) * l;
    double Y = y + sin(a) * l;
    v[0] = x - dx;
    v[1] = y + dy;
    v[2] = x + dx;
    v[3] = y - dy;
    v[4] = X + dx;
    v[5] = Y - dy;
    v[6] = X - dx;
    v[7] = Y + dy;
}

static void lbatchRingSlice(double x, double y, double r, double t, double o, double a ,size_t nv, size_t *i, double *v) {
    for (size_t j = 0; j < QQ - 1; ++j) {
        i[j * 4 + 0] = nv + j * 2 + 0;
        i[j * 4 + 1] = nv + j * 2 + 2;
        i[j * 4 + 2] = nv + j * 2 + 1;
        i[j * 4 + 3] = nv + j * 2 + 3;
    }
    i[QQ * 4 - 4] = nv + 0;
    i[QQ * 4 - 3] = nv + 1;
    i[QQ * 4 - 2] = nv + QQ * 2 - 2;
    i[QQ * 4 - 1] = nv + QQ * 2 - 1;

    double da = (a - o) / (QQ - 1);
    for (size_t j = 0; j < QQ; ++j) {
        double angle = o + da * j;
        double s = sin(angle);
        double c = cos(angle);
        v[j * 4 + 0] = x + c * (r - t / 2);
        v[j * 4 + 1] = y + s * (r - t / 2);
        v[j * 4 + 2] = x + c * (r + t / 2);
        v[j * 4 + 3] = y + s * (r + t / 2);
    }
}
