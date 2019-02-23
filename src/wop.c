#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "wop.h"

#define IS0(x) (fabs(x)<0.0009765625)
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

#define PI 3.1415926535
#define SM 0.99

typedef struct {
    double x, y, a, l;
} Line;

typedef struct {
    double x, y, r, o, a;
} Arc;

typedef struct {
    double x, y, r;
} Circle;

typedef struct {
    bool isArc;
    union {
        Line line;
        Arc arc;
    } c;
} Curve;

static Curve *buildCurve(const char *w);
static void buildCurvePart(char w, double *x, double *y, char *dir, Curve *c);
static void buildLine(Curve *c, Line line, double t);
static void buildArc(Curve *c, Arc arc, double t);
static bool detectCollision(size_t l, const Curve *c);
static bool detectCurveCollision(Curve a, Curve b);
static bool collLineLine(Line a, Line b);
static bool collLineArc(Line a, Arc b);
static bool collArcArc(Arc a, Arc b);
static size_t collCircleCircle(Circle a, Circle b, double *x, double *y);
static double s(double x);
static double ctg(double a);
static double len(double x1, double y1, double x2, double y2);
static WOpRect getRect(const char *w);
static WOpRect linRectInterpolation(WOpRect r0, WOpRect r1, float dt);

bool wOpIsValid(const char *w) {
    Curve *c = buildCurve(w);
    bool collision = detectCollision(strlen(w), c);
    free(c);
    return !collision;
}

static Curve *buildCurve(const char *w) {
    size_t l = strlen(w);
    double x[2] = {0, 0};
    double y[2] = {0, 0};
    char dir = 'R';
    Curve *c = malloc(l * 4 * sizeof(*c));

    for (size_t i = l - 1; i < l; --i) {
        size_t j = l - i - 1;
        buildCurvePart(w[i], x, y, &dir, c + j * 4);
    }

    return c;
}

static void buildCurvePart(char w, double *x, double *y, char *dir, Curve *c) {
    double a = PI / 2 * SM;
    if (*dir == 'U') {
        if (w == 'U') {
            buildArc(c, (Arc){*x - 1, *y, 1, 0, a}, SM);
            *x -= 1;
            *y += 1;
            *dir = 'L';
        } else if (w == 'D') {
            buildArc(c, (Arc){*x + 1, *y, 1, PI - a, a}, SM);
            *x += 1;
            *y += 1;
            *dir = 'R';
        } else {
            buildLine(c, (Line){*x, *y, PI / 2, a}, SM);
            *y += PI / 2;
        }
    } else if (*dir == 'D') {
        if (w == 'U') {
            buildArc(c, (Arc){*x + 1, *y, 1, PI, a}, SM);
            *x += 1;
            *y -= 1;
            *dir = 'R';
        } else if (w == 'D') {
            buildArc(c, (Arc){*x - 1, *y, 1, PI * 2 - a, a}, SM);
            *x -= 1;
            *y -= 1;
            *dir = 'L';
        } else {
            buildLine(c, (Line){*x, *y, PI * 3 / 2, a}, SM);
            *y -= PI / 2;
        }
    } else if (*dir == 'L') {
        if (w == 'U') {
            buildArc(c, (Arc){*x, *y - 1, 1, PI / 2, a}, SM);
            *x -= 1;
            *y -= 1;
            *dir = 'D';
        } else if (w == 'D') {
            buildArc(c, (Arc){*x, *y + 1, 1, PI * 3 / 2 - a, a}, SM);
            *x -= 1;
            *y += 1;
            *dir = 'U';
        } else {
            buildLine(c, (Line){*x, *y, PI, a}, SM);
            *x -= PI / 2;
        }
    } else {
        if (w == 'U') {
            buildArc(c, (Arc){*x, *y + 1, 1, PI * 3 / 2, a}, SM);
            *x += 1;
            *y += 1;
            *dir = 'U';
        } else if (w == 'D') {
            buildArc(c, (Arc){*x, *y - 1, 1, PI / 2 - a, a}, SM);
            *x += 1;
            *y -= 1;
            *dir = 'D';
        } else {
            buildLine(c, (Line){*x, *y, 0, a}, SM);
            *x += PI / 2;
        }
    }
}

static void buildLine(Curve *c, Line line, double t) {
    double x1 = line.x + cos(line.a - PI / 2) * t / 2;
    double y1 = line.y + sin(line.a - PI / 2) * t / 2;
    double x2 = line.x + cos(line.a + PI / 2) * t / 2;
    double y2 = line.y + sin(line.a + PI / 2) * t / 2;
    double x3 = x1 + cos(line.a) * line.l;
    double y3 = y1 + sin(line.a) * line.l;
    c[0] = (Curve){false, .c.line = {x1, y1, line.a, line.l}};
    c[1] = (Curve){false, .c.line = {x2, y2, line.a, line.l}};
    c[2] = (Curve){false, .c.line = {x1, y1, line.a + PI / 2, t}};
    c[3] = (Curve){false, .c.line = {x3, y3, line.a + PI / 2, t}};
}

static void buildArc(Curve *c, Arc arc, double t) {
    double r = arc.r - t / 2;
    double a1 = arc.o;
    double a2 = arc.o + arc.a;
    c[0] = (Curve){false, .c.line={arc.x+cos(a1)*r, arc.y+sin(a1)*r, a1, t}};
    c[1] = (Curve){false, .c.line={arc.x+cos(a2)*r, arc.y+sin(a2)*r, a2, t}};
    c[2] = (Curve){true, .c.arc = {arc.x, arc.y, r, arc.o, arc.a}};
    c[3] = (Curve){true, .c.arc = {arc.x, arc.y, r + t, arc.o, arc.a}};
}

static bool detectCollision(size_t l, const Curve *c) {
    for (size_t i = 0; i < l; ++i) {
        for (size_t j = i + 1; j < l; ++j) {
            for (size_t k = 0; k < 4; ++k) {
                for (size_t g = 0; g < 4; ++g) {
                    if (detectCurveCollision(c[i * 4 + k], c[j * 4 + g])) {
                        return true;
                    }
                }
            }
        }
    }

    Curve x[6];
    x[0] = (Curve){true, .c.arc = {0,  1, SM / 2, 0, PI * 2 * SM}};
    x[1] = (Curve){true, .c.arc = {0, -1, SM / 2, 0, PI * 2 * SM}};
    x[2] = (Curve){false, .c.line = {-99, -SM / 2, 0, 98 + SM}};
    x[3] = (Curve){false, .c.line = {-99,  SM / 2, 0, 98 + SM}};
    x[4] = (Curve){false, .c.line = {-99, -SM / 2, PI / 2, SM}};
    x[5] = (Curve){false, .c.line = {SM - 1, -SM / 2, PI / 2, SM}};

    for (size_t i = 0; i < l * 4; ++i) {
        for (size_t j = 0; j < 6; ++j) {
            if (detectCurveCollision(c[i], x[j])) {
                return true;
            }
        }
    }

    return false;
}

static bool detectCurveCollision(Curve a, Curve b) {
    if (a.isArc) {
        if (b.isArc) {
            if (collArcArc(a.c.arc, b.c.arc)) {
                return true;
            }
        } else {
            if (collLineArc(b.c.line, a.c.arc)) {
                return true;
            }
        }
    } else {
        if (b.isArc) {
            if (collLineArc(a.c.line, b.c.arc)) {
                return true;
            }
        } else {
            if (collLineLine(a.c.line, b.c.line)) {
                return true;
            }
        }
    }
    return false;
}

static bool collLineLine(Line a, Line b) {
    double l1, l2;
    if (IS0(a.a - b.a)) {
        double ax = a.x + cos(a.a) * a.l;
        double ay = a.y + sin(a.a) * a.l;
        double bx = b.x + cos(b.a) * b.l;
        double by = b.y + sin(b.a) * b.l;
        double l = len(a.x, a.y, b.x, b.y);
        double la = len(a.x, a.y, bx, by);
        double lb = len(b.x, b.y, ax, ay);
        if (l > a.l && l > b.l && la > a.l && lb > b.l) {
            return false;
        }
        double x1 = a.x + cos(a.a) * MIN(l, a.l);
        double y1 = a.y + sin(a.a) * MIN(l, a.l);
        double x2 = b.x + cos(b.a) * MIN(l, b.l);
        double y2 = b.y + sin(b.a) * MIN(l, b.l);
        double x3 = a.x + cos(a.a) * MIN(la, a.l);
        double y3 = a.y + sin(a.a) * MIN(la, a.l);
        double x4 = b.x + cos(b.a) * MIN(lb, b.l);
        double y4 = b.y + sin(b.a) * MIN(lb, b.l);
        bool c1 = (IS0(x1 - b.x) && IS0(y1 - b.y));
        bool c2 = (IS0(x2 - a.x) && IS0(y2 - a.y));
        bool c3 = (IS0(x3 - bx) && IS0(y3 - by));
        bool c4 = (IS0(x4 - ax) && IS0(y4 - ay));
        return c1 || c2 || c3 || c4;
    } else if (fabs(cos(a.a)) > fabs(sin(a.a))) {
        l2 = (a.y-b.y+tan(a.a) * (b.x-a.x)) / (sin(b.a)-tan(a.a)*cos(b.a));
        l1 = (b.x - a.x + cos(b.a) * l2) / cos(a.a);
    } else {
        l2 = (a.x-b.x+ctg(a.a) * (b.y-a.y)) / (cos(b.a)-ctg(a.a)*sin(b.a));
        l1 = (b.y - a.y + sin(b.a) * l2) / sin(a.a);
    }
    return l1 >= 0 && l1 <= a.l && l2 >= 0 && l2 <= b.l;
}

static bool collLineArc(Line a, Arc b) {
    double A = 1;
    double B = 2 * (cos(a.a) * (a.x - b.x) + sin(a.a) * (a.y - b.y));
    double C = s(a.x) - 2*a.x*b.x+s(b.x)+s(a.y) - 2*a.y*b.y+s(b.y)-s(b.r);
    double D = s(B) - 4 * A * C;
    size_t n = 0;
    double l[2];
    if (IS0(D)) {
        n = 1;
        l[0] = -B / (2 * A);
    } else if (D > 0) {
        n = 2;
        l[0] = (-B + sqrt(D)) / (2 * A);
        l[1] = (-B - sqrt(D)) / (2 * A);
    }
    for (size_t i = 0; i < n; ++i) {
        double x = a.x + cos(a.a) * l[i];
        double y = a.y + sin(a.a) * l[i];
        double z = atan2(y - b.y, x - b.x);
        z = z < 0 ? z + PI * 2 : z;
        if (l[i] >= 0 && l[i] <= a.l && z >= b.o && z <= b.o + b.a) {
            return true;
        }
    }
    return false;
}

static bool collArcArc(Arc a, Arc b) {
    double x[2], y[2];
    Circle ca = {a.x, a.y, a.r};
    Circle cb = {b.x, b.y, b.r};
    size_t n = collCircleCircle(ca, cb, x, y);
    if (n == 0) {
        return false;
    } else if (n == (size_t)-1) {
        return !(a.o > b.o + b.a || a.o + a.a < b.o);
    } else {
        for (size_t i = 0; i < n; ++i) {
            double a1 = atan2(y[i] - a.y, x[i] - a.x);
            double a2 = atan2(y[i] - b.y, x[i] - b.x);
            a1 = a1 < 0 ? a1 + PI * 2 : a1;
            a2 = a2 < 0 ? a2 + PI * 2 : a2;
            if (a1 >= a.o && a1 <= a.o + a.a && a2 >= b.o && a2 <= b.o + b.a) {
                return true;
            }
        }
    }
    return false;
}

static size_t collCircleCircle(Circle a, Circle b, double *x, double *y) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    if (!IS0(dx) && fabs(dx) > fabs(dy)) {
        double xy = (a.y - b.y) / dx;
        double x0 = (s(a.r)-s(b.r)-s(a.x)+s(b.x)-s(a.y)+s(b.y)) / dx / 2;
        double A = s(xy) + 1;
        double B = 2 * (xy * x0 - xy * a.x - a.y);
        double C = s(x0) - 2 * x0 * a.x + s(a.x) + s(a.y) - s(a.r);
        double D = s(B) - 4 * A * C;
        if (IS0(D)) {
            y[0] = -B / (2 * A);
            x[0] = xy * y[0] + x0;
            return 1;
        } else if (D > 0) {
            y[0] = (-B + sqrt(D)) / (2 * A);
            x[0] = xy * y[0] + x0;
            y[1] = (-B - sqrt(D)) / (2 * A);
            x[1] = xy * y[1] + x0;
            return 2;
        } else {
            return 0;
        }
    } else if (!IS0(dy)) {
        double yx = (a.x - b.x) / dy;
        double y0 = (s(a.r)-s(b.r)-s(a.x)+s(b.x)-s(a.y)+s(b.y)) / dy / 2;
        double A = s(yx) + 1;
        double B = 2 * (yx * y0 - yx * a.y - a.x);
        double C = s(y0) - 2 * y0 * a.y + s(a.x) + s(a.y) - s(a.r);
        double D = s(B) - 4 * A * C;
        if (IS0(D)) {
            x[0] = -B / (2 * A);
            y[0] = yx * x[0] + y0;
            return 1;
        } else if (D > 0) {
            x[0] = (-B + sqrt(D)) / (2 * A);
            y[0] = yx * x[0] + y0;
            x[1] = (-B - sqrt(D)) / (2 * A);
            y[1] = yx * x[1] + y0;
            return 2;
        } else {
            return 0;
        }
    } else if (IS0(a.r - b.r)) {
        return (size_t)-1;
    } else {
        return 0;
    }
}

static double s(double x) {
    return x * x;
}

static double ctg(double a) {
    return cos(a) / sin(a);
}

static double len(double x1, double y1, double x2, double y2) {
    return sqrt(s(x1 - x2) + s(y1 - y2));
}

char *wOpCurrW(const char *wire, char wActive) {
    size_t n = strlen(wire);
    char *w = strcpy(malloc(n + 2), wire);
    w[n + 0] = wActive == 'L' ? '\0' : wActive;
    w[n + 1] = 0;
    return w;
}

char *wOpNextW(const char *wire, char wActive, char action) {
    size_t n = strlen(wire);
    char *w = strcpy(malloc(n + 3), wire);
    if (action == 'L') {
        w[n] = '\0';
    } else if (action == 'R') {
        if (wActive == 'L') {
            w[n + 0] = 'R';
            w[n + 1] = '\0';
        } else {
            w[n + 0] = wActive;
            w[n + 1] = 'R';
            w[n + 2] = '\0';
        }
    } else {
        w[n + 0] = action;
        w[n + 1] = '\0';
    }
    return w;
}

WOpRect wOpGetRect(const char *w0, const char *w1, bool animation, char action, float dt) {
    size_t l0 = strlen(w0);
    char w = l0 ? w0[l0 - 1] : 'L';

    if (w == 'L' && action != 'R') {
        return (WOpRect){0, 0, 0, 0};
    }

    WOpRect r0 = getRect(w0);

    if (!animation || (w == action && w != 'R')) {
        return r0;
    }

    WOpRect r1 = getRect(w1);

    if (action == 'U') {
        if (w == 'D') {
            // TODO 1
            return r0;
        } else { // w == R
            // TODO 2
            return r0;
        }
    } else if (action == 'D') {
        if (w == 'U') {
            // TODO 3
            return r0;
        } else { // w == R
            // TODO 4
            return r0;
        }
    } else if (action == 'L') {
        if (w == 'U') {
            // TODO 5
            return r0;
        } else if (w == 'D') {
            // TODO 6
            return r0;
        } else { // w == R
            return linRectInterpolation(r0, r1, dt);
        }
    } else { // action == R
        return linRectInterpolation(r0, r1, dt);
    }
}

static WOpRect getRect(const char *w) {
    WOpRect r = {0, 0, 0, 0};
    size_t l = strlen(w);
    double x = 0;
    double y = 0;
    char dir = 'R';
    char prevdir = dir;
    for (size_t i = l - 1; i < l; --i) {
        if (dir == 'U') {
            if (w[i] == 'U') {
                x -= 1;
                y += 1;
                dir = 'L';
            } else if (w[i] == 'D') {
                x += 1;
                y += 1;
                dir = 'R';
            } else {
                y += PI / 2;
            }
        } else if (dir == 'D') {
            if (w[i] == 'U') {
                x += 1;
                y -= 1;
                dir = 'R';
            } else if (w[i] == 'D') {
                x -= 1;
                y -= 1;
                dir = 'L';
            } else {
                y -= PI / 2;
            }
        } else if (dir == 'L') {
            if (w[i] == 'U') {
                x -= 1;
                y -= 1;
                dir = 'D';
            } else if (w[i] == 'D') {
                x -= 1;
                y += 1;
                dir = 'U';
            } else {
                x -= PI / 2;
            }
        } else {
            if (w[i] == 'U') {
                x += 1;
                y += 1;
                dir = 'U';
            } else if (w[i] == 'D') {
                x += 1;
                y -= 1;
                dir = 'D';
            } else {
                x += PI / 2;
            }
        }
        float ry = (prevdir == 'U' && w[i] != 'R') ? y + 0.5
                 : (prevdir == 'D' && w[i] != 'R') ? y - 0.5
                 : y;
        float rx = (prevdir == 'R' && w[i] != 'R') ? x + 0.5
                 : (prevdir == 'L' && w[i] != 'R') ? x - 0.5
                 : x;
        prevdir = dir;
        r.x = MIN(r.x, rx);
        r.y = MIN(r.y, ry);
        r.w = MAX(r.w, rx);
        r.h = MAX(r.h, ry);
    }
    r.w = r.x < 0 ? r.w - r.x : r.w;
    r.h = r.y < 0 ? r.h - r.y : r.h;
    return r;
}

static WOpRect linRectInterpolation(WOpRect r0, WOpRect r1, float dt) {
    r0.x += (r1.x - r0.x) * dt;
    r0.y += (r1.y - r0.y) * dt;
    r0.w += (r1.w - r0.w) * dt;
    r0.h += (r1.h - r0.h) * dt;
    return r0;
}
