#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <tgmath.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../lib/lib.h"
#include "main.h"

#define WIN_T "Wire Bending Machine Simulator"
#define OGL_API GLFW_OPENGL_ES_API
#define OGL_V 20
#define VSYNC 1
#define MSAA 16
#define ZOOM 0.25
#define WT 1.0 // Wire thickness
#define CX 0.0 // Center X
#define CY 0.0 // Center Y
#define CCT (WT * 0.05) // Circle Contour Thickness
#define CSR (WT * 0.05) // Circle Screw Radius
#define CSO (WT * 0.5 - WT * 0.15) // Circle Screw Offset from circle
#define CSN 8 // Circle Screw Count
#define CSC (const uint8_t[]){64, 64, 64} // Circle Screw Color
#define WC (const uint8_t[]){255, 255, 255} // Wire Color
#define CC (const uint8_t[]){128, 128, 128} // Circle Color
#define CCC (const uint8_t[]){64, 64, 64} // Circle Contour Color
#define DT 1.0 // Animation duration

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define CLAMP(min,val,max) (MIN((max),MAX((min),(val))))

struct S s;

static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa);
static void draw(float ar);
static void drawBalls(void);
static void drawBall(float cx, float cy, float a);
static void drawActiveWire(void);
static void drawPassiveWire(void);
static void stopAnimation(void);
static void startAnimation(GLFWwindow *win);

int main(void) {
    glfwInit();
    GLFWwindow *win = mkWin(WIN_T, OGL_API, OGL_V, VSYNC, MSAA);
    rInit();
    memset(&s, 0, sizeof(s));
    s.wire.active = 'L';

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int winW, winH;
        glfwGetFramebufferSize(win, &winW, &winH);
        rViewport(0, 0, winW, winH);
        float ar = (float)winW / winH;
        rPipe(ZOOM / ar, ZOOM, 0, 0);

        rClear(0, 0, 0);
        draw(ar);
        glfwSwapBuffers(win);

        stopAnimation();
        startAnimation(win);
    }

    free(s.wire.passive);
    batchDel(&s.b);
    rExit();
    glfwTerminate();
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

static void draw(float ar) {
    batchRect(&s.b, (const float[]){-ar / ZOOM, -WT / 2 + CY, ar / ZOOM + CX, WT}, WC);
    drawBalls();
    drawActiveWire();
    drawPassiveWire();
    rTris(s.b.ni, s.b.i, s.b.v);
    batchClear(&s.b);
}

static void drawBalls(void) {
    if (!s.animation.on) {
        drawBall(CX, CY + WT, 0);
        drawBall(CX, CY - WT, 0);
    } else {
        float dt = s.animation.on ? CLAMP(0, (glfwGetTime() - s.animation.start) / DT, 1) : 0;
        float dt2 = 1 - fabs(1 - dt * 2); // While dt goes [0 -> 1], dt2 goes [0 -> 1 -> 0]
        if (s.animation.action == 'U') {
            drawBall(CX, CY + WT, 0);
            float cx = CX + cos(-PI / 2 + PI / 2 * dt2) * WT * 2;
            float cy = CY + sin(-PI / 2 + PI / 2 * dt2) * WT * 2 + WT;
            drawBall(cx, cy, dt2 * PI);
        } else if (s.animation.action == 'D') {
            float cx = CX + cos(PI / 2 + -PI / 2 * dt2) * WT * 2;
            float cy = CY + sin(PI / 2 + -PI / 2 * dt2) * WT * 2 - WT;
            drawBall(cx, cy, dt2 * -PI);
            drawBall(CX, CY - WT, 0);
        } else if (s.animation.action == 'L') {
            if (s.wire.active == 'L' || s.wire.active == 'R') {
                drawBall(CX, CY + WT, dt * -PI);
                drawBall(CX, CY - WT, dt *  PI);
            } else if (s.wire.active == 'U') {
                drawBall(CX, CY + WT, dt * -PI / 2);
                drawBall(CX, CY - WT, dt *  PI);
            } else if (s.wire.active == 'D') {
                drawBall(CX, CY + WT, dt * -PI);
                drawBall(CX, CY - WT, dt *  PI / 2);
            }
        } else {
            drawBall(CX, CY + WT, dt *  PI);
            drawBall(CX, CY - WT, dt * -PI);
        }
    }
}

static void drawBall(float cx, float cy, float a) {
    float da = PI * 2 / CSN;
    batchCircle(&s.b, cx, cy, WT / 2, QC, CC);
    batchRing(&s.b, cx, cy, WT / 2 - CCT / 2, CCT, QC, CCC);
    for (size_t i = 0; i < CSN; ++i) {
        float x = cos(da * i + a) * CSO + cx;
        float y = sin(da * i + a) * CSO + cy;
        batchCircle(&s.b, x, y, CSR, QCS, CSC);
    }
}

static void drawActiveWire(void) {
    if (!s.animation.on) {
        if (s.wire.active == 'R') {
            batchRect(&s.b, (const float[]){CX, CY - WT / 2, PI / 2 * WT, WT}, WC);
        } else if (s.wire.active == 'U') {
            batchRingSlice(&s.b, CX, CY + WT, WT, WT, -PI/2,  PI/2, QQ, WC);
        } else if (s.wire.active == 'D') {
            batchRingSlice(&s.b, CX, CY - WT, WT, WT,  PI/2, -PI/2, QQ, WC);
        }
    } else {
        float dt = s.animation.on ? CLAMP(0, (glfwGetTime() - s.animation.start) / DT, 1) : 0;
        float dt2 = MIN(dt * 2, 1);
        float dt3 = dt2 * PI / 2;
        if (s.animation.action == 'L') {
            if (s.wire.active == 'R') {
                batchRect(&s.b, (const float[]){CX, CY - WT / 2, (1 - dt) * PI / 2 * WT, WT}, WC);
            } else if (s.wire.active == 'U') {
                batchRingSlice(&s.b, CX, CY + WT, WT, WT, -PI / 2, (1 - dt) *  PI / 2, QQ, WC);
            } else if (s.wire.active == 'D') {
                batchRingSlice(&s.b, CX, CY - WT, WT, WT,  PI / 2, (1 - dt) * -PI / 2, QQ, WC);
            }
        } else if (s.animation.action == 'U') {
            float a = -PI / 2 + dt3;
            if (s.wire.active == 'R') {
                batchRingSlice(&s.b, CX, CY + WT, WT, WT, -PI / 2, dt3, QQ, WC);
                batchLine(&s.b, CX + cos(a) * WT, CY + sin(a) * WT + WT, dt3, (PI / 2 - dt3) * WT, WT, WC);
            } else if (s.wire.active == 'U') {
                batchRingSlice(&s.b, CX, CY + WT, WT, WT, -PI/2,  PI/2, QQ, WC);
            } else if (s.wire.active == 'D') {
                batchRingSlice(&s.b, CX + cos(a) * WT * 2, CY + sin(a) * WT * 2 + WT, WT, WT, PI / 2 + dt3, -PI / 2 + dt3, QQ, WC);
                batchRingSlice(&s.b, CX, CY + WT, WT, WT, -PI / 2, dt3, QQ, WC);
            }
        } else if (s.animation.action == 'D') {
            float a = PI / 2 - dt3;
            if (s.wire.active == 'R') {
                batchRingSlice(&s.b, CX, CY - WT, WT, WT, PI / 2, -dt3, QQ, WC);
                batchLine(&s.b, CX + cos(a) * WT, CY + sin(a) * WT - WT, -dt3, (PI / 2 - dt3) * WT, WT, WC);
            } else if (s.wire.active == 'U') {
                batchRingSlice(&s.b, CX + cos(a) * WT * 2, CY + sin(a) * WT * 2 - WT, WT, WT, -PI / 2 - dt3, PI / 2 - dt3, QQ, WC);
                batchRingSlice(&s.b, CX, CY - WT, WT, WT, PI / 2, -dt3, QQ, WC);
            } else if (s.wire.active == 'D') {
                batchRingSlice(&s.b, CX, CY - WT, WT, WT,  PI/2, -PI/2, QQ, WC);
            }
        } else {
            batchRect(&s.b, (const float[]){CX, CY - WT / 2, dt * PI / 2 * WT, WT}, WC);
        }
    }
}

static void drawPassiveWire(void) {
    if (!s.animation.on) {
        char dir = s.wire.active;
        double x, y;
        if (s.wire.active == 'U') {
            x = y = WT;
        } else if (s.wire.active == 'D') {
            x = WT;
            y = -WT;
        } else {
            x = PI / 2 * WT;
            y = 0;
        }
        for (size_t i = s.wire.n - 1; i < s.wire.n; --i) {
            if (dir == 'U') {
                if (s.wire.passive[i] == 'U') {
                    batchRingSlice(&s.b, CX + x - WT, CY + y, WT, WT, 0, PI / 2, QQ, WC);
                    x -= WT;
                    y += WT;
                    dir = 'L';
                } else if (s.wire.passive[i] == 'D') {
                    batchRingSlice(&s.b, CX + x + WT, CY + y, WT, WT, PI, -PI / 2, QQ, WC);
                    x += WT;
                    y += WT;
                    dir = 'R';
                } else {
                    batchLine(&s.b, CX + x, CY + y, PI / 2, PI / 2 * WT, WT, WC);
                    y += PI / 2 * WT;
                }
            } else if (dir == 'D') {
                if (s.wire.passive[i] == 'U') {
                    batchRingSlice(&s.b, CX + x + WT, CY + y, WT, WT, PI, PI / 2, QQ, WC);
                    x += WT;
                    y -= WT;
                    dir = 'R';
                } else if (s.wire.passive[i] == 'D') {
                    batchRingSlice(&s.b, CX + x - WT, CY + y, WT, WT, 0, -PI / 2, QQ, WC);
                    x -= WT;
                    y -= WT;
                    dir = 'L';
                } else {
                    batchLine(&s.b, CX + x, CY + y, -PI / 2, PI / 2 * WT, WT, WC);
                    y -= PI / 2 * WT;
                }
            } else if (dir == 'L') {
                if (s.wire.passive[i] == 'U') {
                    batchRingSlice(&s.b, CX + x, CY + y - WT, WT, WT, PI / 2, PI / 2, QQ, WC);
                    x -= WT;
                    y -= WT;
                    dir = 'D';
                } else if (s.wire.passive[i] == 'D') {
                    batchRingSlice(&s.b, CX + x, CY + y + WT, WT, WT, -PI / 2, -PI / 2, QQ, WC);
                    x -= WT;
                    y += WT;
                    dir = 'U';
                } else {
                    batchLine(&s.b, CX + x, CY + y, 0, -PI / 2 * WT, WT, WC);
                    x -= PI / 2 * WT;
                }
            } else {
                if (s.wire.passive[i] == 'U') {
                    batchRingSlice(&s.b, CX + x, CY + y + WT, WT, WT, -PI / 2, PI / 2, QQ, WC);
                    x += WT;
                    y += WT;
                    dir = 'U';
                } else if (s.wire.passive[i] == 'D') {
                    batchRingSlice(&s.b, CX + x, CY + y - WT, WT, WT, PI / 2, -PI / 2, QQ, WC);
                    x += WT;
                    y -= WT;
                    dir = 'D';
                } else {
                    batchLine(&s.b, CX + x, CY + y, 0, PI / 2 * WT, WT, WC);
                    x += PI / 2 * WT;
                }
            }
        }
    }
    // TODO 1
}

static void stopAnimation(void) {
    if (!s.animation.on || s.animation.start + DT >= glfwGetTime()) {
        return;
    }
    s.animation.on = 0;
    if (s.animation.action == 'L') {
        s.wire.active = s.wire.n > 0 ? s.wire.passive[--s.wire.n] : 'L';
        if (s.wire.m) {
            s.wire.passive[s.wire.n] = 0;
        }
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

    if (s.animation.on || !(left || right || up || down)) {
        return;
    }

    s.animation.on = 1;
    s.animation.start = glfwGetTime();

    if (left) {
        if (s.wire.active == 'L') {
            s.animation.on = 0;
        }
        s.animation.action = 'L';
    } else if (right) {
        if (s.wire.active != 'L') {
            if (s.wire.n <= s.wire.m) {
                s.wire.m = s.wire.m ? 64 : s.wire.m * 2;
                s.wire.passive = realloc(s.wire.passive, s.wire.m + 1);
            }
            s.wire.passive[s.wire.n++] = s.wire.active;
            s.wire.passive[s.wire.n] = 0;
        }
        s.animation.action = 'R';
        s.wire.active = 'L';
    } else if (up) {
        s.animation.action = 'U';
    } else if (down) {
        s.animation.action = 'D';
    }
}
