#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <tgmath.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../lib/lib.h"
#include "wop.h"

#define PI 3.1415926535

#define QQ 40 // Quality of Quarter rings
#define QC 40 // Quality of Circle
#define QCS 8 // Quality of Circle Screw

#define WIN_T "Wire Bending Machine Simulator"
#define OGL_API GLFW_OPENGL_ES_API
#define OGL_V 20
#define VSYNC 1
#define MSAA 16
#define ZOOM 0.25

#define CCT (0.05) // Circle Contour Thickness
#define CSR (0.05) // Circle Screw Radius
#define CSO (0.5 - 0.15) // Circle Screw Offset from circle
#define CSN 8 // Circle Screw Count
#define CMAXX (-0.5) // Camera Maximum X
#define CMAXY (-1.5) // Camera Maximum Y
#define CMINW(rx) (2.5 - rx) // Camera Minimum W
#define CMINH(ry) (1.5 - ry) // Camera Minimum H

#define CSC (const uint8_t[]){64, 64, 64} // Circle Screw Color
#define WC (const uint8_t[]){255, 255, 255} // Wire Color
#define CC (const uint8_t[]){128, 128, 128} // Circle Color
#define CCC (const uint8_t[]){64, 64, 64} // Circle Contour Color

float DT = 1.0F; // Standart Animation duration

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define CLAMP(min,val,max) (MIN((max),MAX((min),(val))))

static struct S {
    Batch b;
    struct {
        bool on;
        double start;
        char action;
    } animation;
    struct {
        size_t n, m;
        char active, *passive;
    } wire;
} s;

static void initS(void);
static void exitS(void);
static void loop(GLFWwindow *win);
static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa);
static void draw(int winW, int winH);
static void setupCameraAndDrawDeadWire(int winW, int winH);
static float setMinCamRect(WOpRect r, float ar);
static void drawBalls(void);
static void drawBall(float cx, float cy, float a);
static void drawActiveWire(void);
static void drawPassiveWire(void);
static void drawPassiveStaticWire(const float *matrix);
static void stopAnimation(void);
static void startAnimation(GLFWwindow *win);
static bool wireWillBeValid(char action);

int main(int argc, char *argv[]) {
    //Check Arguments
    for (int i = 0; i < argc; i++) {
        if (argc > i+1 && strcmp(argv[i],"--anim-duration") == 0) {
            if (atof(argv[i+1]) != 0) {
                DT = atof(argv[i+1]);
            }
        }
    }

    glfwInit();
    GLFWwindow *win = mkWin(WIN_T, OGL_API, OGL_V, VSYNC, MSAA);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    rInit();
    initS();

    loop(win);

    exitS();
    rExit();
    glfwTerminate();
}

static void initS(void) {
    memset(&s, 0, sizeof(s));
    s.wire.m = 64;
    s.wire.active = 'L';
    s.wire.passive = calloc(s.wire.m, 1);
}

static void exitS(void) {
    free(s.wire.passive);
    batchDel(&s.b);
}

static void loop(GLFWwindow *win) {
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int winW, winH;
        glfwGetFramebufferSize(win, &winW, &winH);

        rClear(0, 0, 0);
        draw(winW, winH);
        glfwSwapBuffers(win);

        stopAnimation();
        startAnimation(win);

        if (glfwGetKey(win, GLFW_KEY_ESCAPE) || glfwGetKey(win, GLFW_KEY_Q)) {
            glfwSetWindowShouldClose(win, true);
        }
    }
}

static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa) {
    glfwWindowHint(GLFW_CLIENT_API, api);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, v / 10);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, v % 10);
    glfwWindowHint(GLFW_SAMPLES, aa);

    GLFWmonitor *mon = glfwGetPrimaryMonitor();
    const GLFWvidmode *vm = glfwGetVideoMode(mon);
    glfwWindowHint(GLFW_RED_BITS, vm->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, vm->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, vm->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, vm->refreshRate);

    GLFWwindow *win = glfwCreateWindow(vm->width, vm->height, t, mon, NULL);
    glfwMakeContextCurrent(win);
    glfwSwapInterval(vs);

    return win;
}

static void draw(int winW, int winH) {
    setupCameraAndDrawDeadWire(winW, winH);
    drawBalls();
    drawActiveWire();
    drawPassiveWire();
    rTris(s.b.ni, s.b.i, s.b.v);
    batchClear(&s.b);
}

static void setupCameraAndDrawDeadWire(int winW, int winH) {
    float dt = s.animation.on ? CLAMP(0, (glfwGetTime() - s.animation.start) / DT, 1) : 0;
    char *w0 = wOpCurrW(s.wire.passive, s.wire.active);
    char *w1 = wOpNextW(s.wire.passive, s.wire.active, s.animation.action);
    WOpRect r = wOpGetRect(w0, w1, s.animation.on, s.animation.action, dt);
    free(w1);
    free(w0);

    r.x *= 1;
    r.y *= 1;
    r.w *= 1;
    r.h *= 1;
    if (CMAXX < r.x) {
        r.w += r.x - CMAXX;
        r.x = CMAXX;
    }
    if (CMAXY < r.y) {
        r.h += r.y - CMAXY;
        r.y = CMAXY;
    }
    r.w = MAX(CMINW(r.x), r.w);
    r.h = MAX(CMINH(r.y), r.h);

    rViewport(0, 0, winW, winH);
    float swl = setMinCamRect(r, (float)winW / (float)winH);
    batchRect(&s.b, (const float[]){-swl, -0.5, swl, 1}, WC);
}

static float setMinCamRect(WOpRect r, float ar) {
    float zx = (2 * ar) / (r.w + 0.001);
    float zy = 2.0 / (r.h + 0.001);
    if (zx < zy) {
        float cy = r.y + r.h / 2;
        rPipe(zx / ar, zx, -1 - r.x * (zx / ar), -cy * zx);
        return r.x < 0 ? -r.x : r.x;
    } else {
        float cx = r.x + r.w / 2;
        rPipe(zy / ar, zy, -cx * (zx / ar), -1 - r.y * zy);
        return (2 * ar) / zy;
    }
}

static void drawBalls(void) {
    if (!s.animation.on) {
        drawBall(0,  1, 0);
        drawBall(0, -1, 0);
    } else {
        float dt = s.animation.on ? CLAMP(0, (glfwGetTime() - s.animation.start) / DT, 1) : 0;
        float dt2 = 1 - fabs(1 - dt * 2); // While dt goes [0 -> 1], dt2 goes [0 -> 1 -> 0]
        if (s.animation.action == 'U') {
            drawBall(0, 1, 0);
            float cx = cos(-PI / 2 + PI / 2 * dt2) * 2;
            float cy = sin(-PI / 2 + PI / 2 * dt2) * 2 + 1;
            drawBall(cx, cy, dt2 * PI);
        } else if (s.animation.action == 'D') {
            float cx = cos(PI / 2 + -PI / 2 * dt2) * 2;
            float cy = sin(PI / 2 + -PI / 2 * dt2) * 2 - 1;
            drawBall(cx, cy, dt2 * -PI);
            drawBall(0, -1, 0);
        } else if (s.animation.action == 'L') {
            if (s.wire.active == 'L' || s.wire.active == 'R') {
                drawBall(0, 1, dt * -PI);
                drawBall(0, -1, dt *  PI);
            } else if (s.wire.active == 'U') {
                drawBall(0, 1, dt * -PI / 2);
                drawBall(0, -1, dt *  PI);
            } else if (s.wire.active == 'D') {
                drawBall(0, 1, dt * -PI);
                drawBall(0, -1, dt *  PI / 2);
            }
        } else {
            drawBall(0, 1, dt *  PI);
            drawBall(0, -1, dt * -PI);
        }
    }
}

static void drawBall(float cx, float cy, float a) {
    float da = PI * 2 / CSN;
    batchCircle(&s.b, cx, cy, 0.5, a, QC, CC);
    batchRing(&s.b, cx, cy, 0.5 - CCT / 2, CCT, a, QC, CCC);
    for (size_t i = 0; i < CSN; ++i) {
        float x = cos(da * i + a) * CSO + cx;
        float y = sin(da * i + a) * CSO + cy;
        batchCircle(&s.b, x, y, CSR, a, QCS, CSC);
    }
}

static void drawActiveWire(void) {
    if (!s.animation.on) {
        if (s.wire.active == 'R') {
            batchRect(&s.b, (const float[]){0, -0.5, PI / 2, 1}, WC);
        } else if (s.wire.active == 'U') {
            batchRingSlice(&s.b, 0, 1, 1, 1, -PI/2,  PI/2, QQ, WC);
        } else if (s.wire.active == 'D') {
            batchRingSlice(&s.b, 0, -1, 1, 1,  PI/2, -PI/2, QQ, WC);
        }
    } else {
        float dt = s.animation.on ? CLAMP(0, (glfwGetTime() - s.animation.start) / DT, 1) : 0;
        float dt2 = MIN(dt * 2, 1);
        float dt3 = dt2 * PI / 2;
        if (s.animation.action == 'L') {
            if (s.wire.active == 'R') {
                batchRect(&s.b, (const float[]){0, -0.5, (1 - dt) * PI / 2, 1}, WC);
            } else if (s.wire.active == 'U') {
                batchRingSlice(&s.b, 0, 1, 1, 1, -PI / 2, (1 - dt) *  PI / 2, QQ, WC);
            } else if (s.wire.active == 'D') {
                batchRingSlice(&s.b, 0, -1, 1, 1,  PI / 2, (1 - dt) * -PI / 2, QQ, WC);
            }
        } else if (s.animation.action == 'U') {
            float a = -PI / 2 + dt3;
            if (s.wire.active == 'R') {
                batchRingSlice(&s.b, 0, 1, 1, 1, -PI / 2, dt3, QQ, WC);
                batchLine(&s.b, cos(a), sin(a) + 1, dt3, (PI / 2 - dt3), 1, WC);
            } else if (s.wire.active == 'U') {
                batchRingSlice(&s.b, 0, 1, 1, 1, -PI/2,  PI/2, QQ, WC);
            } else if (s.wire.active == 'D') {
                batchRingSlice(&s.b, cos(a) * 2, sin(a) * 2 + 1, 1, 1, PI / 2 + dt3, -PI / 2 + dt3, QQ, WC);
                batchRingSlice(&s.b, 0, 1, 1, 1, -PI / 2, dt3, QQ, WC);
            }
        } else if (s.animation.action == 'D') {
            float a = PI / 2 - dt3;
            if (s.wire.active == 'R') {
                batchRingSlice(&s.b, 0, -1, 1, 1, PI / 2, -dt3, QQ, WC);
                batchLine(&s.b, cos(a), sin(a) - 1, -dt3, (PI / 2 - dt3), 1, WC);
            } else if (s.wire.active == 'U') {
                batchRingSlice(&s.b, cos(a) * 2, sin(a) * 2 - 1, 1, 1, -PI / 2 - dt3, PI / 2 - dt3, QQ, WC);
                batchRingSlice(&s.b, 0, -1, 1, 1, PI / 2, -dt3, QQ, WC);
            } else if (s.wire.active == 'D') {
                batchRingSlice(&s.b, 0, -1, 1, 1,  PI/2, -PI/2, QQ, WC);
            }
        } else {
            batchRect(&s.b, (const float[]){0, -0.5, dt * PI / 2, 1}, WC);
        }
    }
}

static void drawPassiveWire(void) {
    if (!s.animation.on) {
        drawPassiveStaticWire(NULL);
    } else {
        float m0[9], m1[9], m2[9];
        float dt = CLAMP(0, (glfwGetTime() - s.animation.start) / DT, 1);
        float dt2 = MIN(dt * 2, 1);
        if (s.animation.action == 'U') {
            if (s.wire.active == 'U') {
                drawPassiveStaticWire(NULL);
            } else if (s.wire.active == 'D') {
                matTrans(m0, -1, 1);
                matRot(m1, PI * dt2);
                matMul(m2, m1, m0);
                float dx = cos(PI * dt2) + cos(-PI / 2 + PI / 2 * dt2) * 2;
                float dy = sin(PI * dt2) + sin(-PI / 2 + PI / 2 * dt2) * 2 + 1;
                matTrans(m1, dx, dy);
                matMul(m0, m1, m2);
                drawPassiveStaticWire(m0);
            } else if (s.wire.active == 'R') {
                matTrans(m0, -PI / 2, 0);
                matRot(m1, PI / 2 * dt2);
                matMul(m2, m1, m0);
                float dx = cos(PI / 2 * dt2) * PI / 2 * (1 - dt2) + cos(-PI / 2 + PI / 2 * dt2);
                float dy = sin(PI / 2 * dt2) * PI / 2 * (1 - dt2) + sin(-PI / 2 + PI / 2 * dt2) + 1;
                matTrans(m1, dx, dy);
                matMul(m0, m1, m2);
                drawPassiveStaticWire(m0);
            }
        } else if (s.animation.action == 'D') {
            if (s.wire.active == 'U') {
                matTrans(m0, -1, -1);
                matRot(m1, -PI * dt2);
                matMul(m2, m1, m0);
                float dx = cos(-PI * dt2) + cos(PI / 2 + -PI / 2 * dt2) * 2;
                float dy = sin(-PI * dt2) + sin(PI / 2 + -PI / 2 * dt2) * 2 - 1;
                matTrans(m1, dx, dy);
                matMul(m0, m1, m2);
                drawPassiveStaticWire(m0);
            } else if (s.wire.active == 'D') {
                drawPassiveStaticWire(NULL);
            } else if (s.wire.active == 'R') {
                matTrans(m0, -PI / 2, 0);
                matRot(m1, -PI / 2 * dt2);
                matMul(m2, m1, m0);
                float dx = cos(-PI / 2 * dt2) * PI / 2 * (1 - dt2) + cos(PI / 2 + -PI / 2 * dt2);
                float dy = sin(-PI / 2 * dt2) * PI / 2 * (1 - dt2) + sin(PI / 2 + -PI / 2 * dt2) - 1;
                matTrans(m1, dx, dy);
                matMul(m0, m1, m2);
                drawPassiveStaticWire(m0);
            }
        } else if (s.animation.action == 'L') {
            if (s.wire.active == 'U') {
                matTrans(m0, -1, -1);
                matRot(m1, -PI / 2 * dt);
                matMul(m2, m1, m0);
                matTrans(m1, cos(-PI / 2 * dt), sin(-PI / 2 * dt) + 1);
                matMul(m0, m1, m2);
                drawPassiveStaticWire(m0);
            } else if (s.wire.active == 'D') {
                matTrans(m0, -1, 1);
                matRot(m1, PI / 2 * dt);
                matMul(m2, m1, m0);
                matTrans(m1, cos(PI / 2 * dt), sin(PI / 2 * dt) - 1);
                matMul(m0, m1, m2);
                drawPassiveStaticWire(m0);
            } else if (s.wire.active == 'R') {
                matTrans(m0, -PI / 2 * dt, 0);
                drawPassiveStaticWire(m0);
            }
        } else {
            matTrans(m0, PI / 2 * dt, 0);
            drawPassiveStaticWire(m0);
        }
    }
}

static void drawPassiveStaticWire(const float *matrix) {
    size_t oldnv = s.b.nv;
    char dir = s.wire.active;
    float x, y;
    if (s.wire.active == 'U') {
        x = y = 1;
    } else if (s.wire.active == 'D') {
        x = 1;
        y = -1;
    } else if (s.wire.active == 'L') {
        x = y = 0;
        dir = 'R';
    } else {
        x = PI / 2;
        y = 0;
    }
    for (size_t i = s.wire.n - 1; i < s.wire.n; --i) {
        if (dir == 'U') {
            if (s.wire.passive[i] == 'U') {
                batchRingSlice(&s.b, x - 1, y, 1, 1, 0, PI / 2, QQ, WC);
                x -= 1;
                y += 1;
                dir = 'L';
            } else if (s.wire.passive[i] == 'D') {
                batchRingSlice(&s.b, x + 1, y, 1, 1, PI, -PI / 2, QQ, WC);
                x += 1;
                y += 1;
                dir = 'R';
            } else {
                batchLine(&s.b, x, y, PI / 2, PI / 2, 1, WC);
                y += PI / 2;
            }
        } else if (dir == 'D') {
            if (s.wire.passive[i] == 'U') {
                batchRingSlice(&s.b, x + 1, y, 1, 1, PI, PI / 2, QQ, WC);
                x += 1;
                y -= 1;
                dir = 'R';
            } else if (s.wire.passive[i] == 'D') {
                batchRingSlice(&s.b, x - 1, y, 1, 1, 0, -PI / 2, QQ, WC);
                x -= 1;
                y -= 1;
                dir = 'L';
            } else {
                batchLine(&s.b, x, y, -PI / 2, PI / 2, 1, WC);
                y -= PI / 2;
            }
        } else if (dir == 'L') {
            if (s.wire.passive[i] == 'U') {
                batchRingSlice(&s.b, x, y - 1, 1, 1, PI / 2, PI / 2, QQ, WC);
                x -= 1;
                y -= 1;
                dir = 'D';
            } else if (s.wire.passive[i] == 'D') {
                batchRingSlice(&s.b, x, y + 1, 1, 1, -PI / 2, -PI / 2, QQ, WC);
                x -= 1;
                y += 1;
                dir = 'U';
            } else {
                batchLine(&s.b, x, y, 0, -PI / 2, 1, WC);
                x -= PI / 2;
            }
        } else {
            if (s.wire.passive[i] == 'U') {
                batchRingSlice(&s.b, x, y + 1, 1, 1, -PI / 2, PI / 2, QQ, WC);
                x += 1;
                y += 1;
                dir = 'U';
            } else if (s.wire.passive[i] == 'D') {
                batchRingSlice(&s.b, x, y - 1, 1, 1, PI / 2, -PI / 2, QQ, WC);
                x += 1;
                y -= 1;
                dir = 'D';
            } else {
                batchLine(&s.b, x, y, 0, PI / 2, 1, WC);
                x += PI / 2;
            }
        }
    }

    if (matrix) {
        for (size_t i = oldnv; i < s.b.nv; ++i) {
            float mv[3];
            float v[3] = {s.b.v[i].x, s.b.v[i].y, 1};
            matMulVec(mv, matrix, v);
            s.b.v[i].x = mv[0];
            s.b.v[i].y = mv[1];
        }
    }
}

static void stopAnimation(void) {
    if (!s.animation.on || s.animation.start + DT >= glfwGetTime()) {
        return;
    }
    s.animation.on = false;
    if (s.animation.action == 'L') {
        s.wire.active = s.wire.n > 0 ? s.wire.passive[--s.wire.n] : 'L';
        s.wire.passive[s.wire.n] = '\0';
    } else if (s.animation.action == 'R') {
        s.wire.active = 'R';
    } else if (s.animation.action == 'U' && s.wire.active != 'L') {
        s.wire.active = 'U';
    } else if (s.animation.action == 'D' && s.wire.active != 'L') {
        s.wire.active = 'D';
    }
}

static void startAnimation(GLFWwindow *win) {
    int left = glfwGetKey(win, GLFW_KEY_LEFT);
    int right = glfwGetKey(win, GLFW_KEY_RIGHT);
    int up = glfwGetKey(win, GLFW_KEY_UP);
    int down = glfwGetKey(win, GLFW_KEY_DOWN);
    char action = left ? 'L' : right ? 'R' : up ? 'U' : down ? 'D' : '\0';

    if (s.animation.on || !action || (left && s.wire.active == 'L') || !wireWillBeValid(action)) {
        return;
    }

    s.animation.on = true;
    s.animation.start = glfwGetTime();

    if (s.wire.n >= s.wire.m) {
        s.wire.m = s.wire.m * 2;
        s.wire.passive = realloc(s.wire.passive, s.wire.m + 1);
    }

    s.animation.action = action;

    if (right && s.wire.active != 'L') {
        s.wire.passive[s.wire.n++] = s.wire.active;
        s.wire.passive[s.wire.n] = '\0';
        s.wire.active = 'L';
    }
}

static bool wireWillBeValid(char action) {
    char *w = wOpNextW(s.wire.passive, s.wire.active, action);
    bool valid = wOpIsValid(w);
    free(w);
    return valid;
}
