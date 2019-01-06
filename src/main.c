#include "../lib/lib.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <math.h>

#define MIN(x,y) (x)<(y)?(x):(y)
#define MAX(x,y) (x)>(y)?(x):(y)
#define CLAMP(min,val,max)  (MIN(max,MAX(min,val)))

#define WIN_T "Wire Bending Machine Simulator"
#define OGL_API GLFW_OPENGL_ES_API
#define OGL_V 20
#define VSYNC 1
#define MSAA 16
#define WIRE_COLOR (uint8_t[]){255,255,255}
#define CIRCLE_COLOR  (uint8_t[]){128,128,128}
#define ANIMATION_DURATION 1.0
#define ZOOM 0.25
#define CIRCLE_QUALITY 40
#define PI 3.14159265358979
#define MAX_WIRE_LEN 1000
enum {UP, DOWN, LEFT, RIGHT};

static struct {
    int animating, wire[MAX_WIRE_LEN], wireLen, action;
    double animationStart;
    Batch b;
} s = {.animating = 0, .wireLen = 0, .b = {0,0,0,0,NULL,NULL}};

static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa);
static void renewGlobalBatch(void);
static void stopAnimation(void);
static void batchStatic(Batch *b, float ar);
static void batchStaticActiveWire(Batch *b);
static void batchAnimated(Batch *b, float progress, float ar);
static void reactToInput(GLFWwindow *win);

int main(void) {
    Batch b = batchNew();
    glfwInit();
    GLFWwindow *win = mkWin(WIN_T, OGL_API, OGL_V, VSYNC, MSAA);
    rInit();

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        float ar = (float)w / (float)h;
        rClear(0, 0, 0);
        rViewport(0, 0, w, h);
        rPipe(ZOOM / ar, ZOOM, 0, 0);

        if (s.animating) {
            float progress = (glfwGetTime() - s.animationStart) / ANIMATION_DURATION;
            progress = progress < 0 ? 0 : progress > 1 ? 1 : progress;
            if (progress >= 1) {
                renewGlobalBatch();
                stopAnimation();
            } else {
                batchAnimated(&b, progress, ar);
            }
        }
        if (!s.animating) {
            batchStatic(&b, ar);
        }
        batchDraw(&b);
        batchClear(&b);

        glfwSwapBuffers(win);
        reactToInput(win);
    }

    rExit();
    glfwTerminate();
    batchDel(&b);
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

static void renewGlobalBatch(void) {
    int wire = s.wireLen > 0 ? s.wire[s.wireLen - 1] : LEFT;
    if (wire == LEFT) {
        return;
    }

    float m0[9], m1[9], m2[9];

    if ((s.action == UP && wire != UP) || (s.action == DOWN && wire != DOWN)) {
        float a = (wire == RIGHT) ? PI / 2 : PI;
        a = (s.action == UP) ? a : -a;

        matTrans(m0, (wire == RIGHT) ? -PI/2 - 0.5 : -1, 0);
        matRot(m1, a);
        matMul(m2, m1, m0);
        matTrans(m1, 1, wire != RIGHT ? 0 : s.action == UP ? 1.5 : -1.5);
        matMul(m0, m1, m2);
    } else if (s.action == LEFT && s.wireLen > 1) {
        if (s.wire[s.wireLen - 2] == UP || s.wire[s.wireLen - 2] == DOWN) {
            batchClearRingSlice(&s.b, CIRCLE_QUALITY);
        } else {
            batchClearLine(&s.b);
        }
        if (wire == RIGHT) {
            matTrans(m0, -PI/2, 0);
        } else {
            float m0[9], m1[9], m2[9];
            matTrans(m0, 1, wire == UP ? -1 : 1);
            matRot(m1, wire == UP ? PI/2*3 : PI/2);
            matMul(m2, m1, m0);
            matTrans(m1, 0, wire == UP ? 2 : -2);
            matMul(m0, m1, m2);
        }
    } else if (s.action == RIGHT) {
        batchStaticActiveWire(&s.b);
        matTrans(m0, PI/2, 0);
    }

    for (size_t i = 0; i < s.b.nv; ++i) {
        float mv[3], v[3] = {s.b.v[i].x, s.b.v[i].y, 1};
        matMulVec(mv, m0, v);
        s.b.v[i].x = mv[0];
        s.b.v[i].y = mv[1];
    }
}

static void stopAnimation(void) {
    s.animating = 0;
    if (s.action == UP && s.wireLen > 0) {
        s.wire[s.wireLen - 1] = UP;
    } else if (s.action == DOWN && s.wireLen > 0) {
        s.wire[s.wireLen - 1] = DOWN;
    } else if (s.action == RIGHT && s.wireLen < MAX_WIRE_LEN) {
        s.wire[s.wireLen++] = RIGHT;
    } else if (s.action == LEFT && s.wireLen > 0) {
        --s.wireLen;
    }
}

static void batchStatic(Batch *b, float ar) {
    batchLine(b, -ar / ZOOM, 0, 0, ar / ZOOM, 1, WIRE_COLOR);
    batchStaticActiveWire(b);
    batchCircle(b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
    batchCircle(b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
    batchDraw(&s.b);
}

static void batchStaticActiveWire(Batch *b) {
    if (s.wireLen > 0 && s.wire[s.wireLen - 1] == UP) {
        batchRingSlice(b, 0, 1, 1, 1,-PI/2, PI/2, CIRCLE_QUALITY, WIRE_COLOR);
    } else if (s.wireLen > 0 && s.wire[s.wireLen - 1] == DOWN) {
        batchRingSlice(b, 0,-1, 1, 1, PI/2,-PI/2, CIRCLE_QUALITY, WIRE_COLOR);
    } else if (s.wireLen > 0 && s.wire[s.wireLen - 1] == RIGHT) {
        batchLine(b, 0, 0, 0, PI/2, 1, WIRE_COLOR);
    }
}

static void batchAnimated(Batch *b, float progress, float ar) {
    // TODO: refactor this steaming pile of shit
    batchLine(b, -ar / ZOOM, 0, 0, ar / ZOOM, 1, WIRE_COLOR);
    float m[9], v1[3] = {0, -2, 1}, v2[3] = {0, 2, 1}, v3[3] = {0, -1, 1}, v4[3] = {0, 1, 1}, mv1[3], mv2[3], mv3[3], mv4[3];
    matRot(m,  (1 - fabsf(progress * 2 - 1)) * PI / 2);
    matMulVec(mv1, m, v1);
    matMulVec(mv3, m, v3);
    matRot(m,  (1 - fabsf(progress * 2 - 1)) * -PI / 2);
    matMulVec(mv2, m, v2);
    matMulVec(mv4, m, v4);
    mv1[1] += 1;
    mv3[1] += 1;
    mv2[1] -= 1;
    mv4[1] -= 1;
    int wire = s.wireLen > 0 ? s.wire[s.wireLen - 1] : LEFT;
    switch (s.action) {

        case UP:
            switch (wire) {
                case UP:
                    batchRingSlice(b, 0, 1, 1, 1, -PI / 2, PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                    break;
                case DOWN:
                    batchRingSlice(b, mv1[0], mv1[1], 1, 1, PI/2+CLAMP(0, progress * 2, 1)*PI/2, -PI/2+CLAMP(0, progress * 2, 1)*PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                    batchRingSlice(b, 0, 1, 1, 1, -PI / 2, CLAMP(0, progress * 2, 1) * PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                    break;
                case RIGHT:
                    batchRingSlice(b, 0, 1, 1, 1, -PI / 2, CLAMP(0, progress * 2, 1) * PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                    batchLine(b, mv3[0], mv3[1], CLAMP(0, progress * 2, 1) *  PI / 2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, 1, WIRE_COLOR);
                    break;
                case LEFT:
                    break;
            }
            batchCircle(b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            batchCircle(b, mv1[0], mv1[1], 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            break;

        case DOWN:
            switch (wire) {
                case UP:
                    batchRingSlice(b, mv2[0], mv2[1], 1, 1, -PI/2-CLAMP(0, progress * 2, 1)*PI/2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                    batchRingSlice(b, 0,-1, 1, 1, PI/2, CLAMP(0, progress * 2, 1) * -PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                    break;
                case DOWN:
                    batchRingSlice(b, 0,-1, 1, 1, PI/2,-PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                    break;
                case RIGHT:
                    batchRingSlice(b, 0,-1, 1, 1, PI/2, CLAMP(0, progress * 2, 1) * -PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                    batchLine(b, mv4[0], mv4[1], CLAMP(0, progress * 2, 1) * -PI / 2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, 1, WIRE_COLOR);
                    break;
                case LEFT:
                    break;
            }
            batchCircle(b, mv2[0], mv2[1], 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            batchCircle(b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            break;

        case RIGHT:
            batchLine(b, 0, 0, 0, progress * PI / 2, 1, WIRE_COLOR);
            batchCircle(b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            batchCircle(b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            break;

        case LEFT:
            switch (wire) {
                case UP:
                    batchRingSlice(b, 0, 1, 1, 1,-PI/2, (1-progress)*PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                    break;
                case DOWN:
                    batchRingSlice(b, 0,-1, 1, 1, PI/2,(1-progress)*-PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                    break;
                case RIGHT:
                    batchLine(b, 0, 0, 0, PI / 2 - progress * PI / 2, 1, WIRE_COLOR);
                    break;
                case LEFT:
                    break;
            }
            batchCircle(b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            batchCircle(b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
            break;
    }
}

static void reactToInput(GLFWwindow *win) {
    int up = glfwGetKey(win, GLFW_KEY_UP);
    int down = glfwGetKey(win, GLFW_KEY_DOWN);
    int left = glfwGetKey(win, GLFW_KEY_LEFT);
    int right = glfwGetKey(win, GLFW_KEY_RIGHT);
    if (!s.animating && (up || down || left || right)) {
        s.animating = 1;
        s.animationStart = glfwGetTime();
        if (up) {
            s.action = UP;
        } else if (down) {
            s.action = DOWN;
        } else if (left) {
            s.action = LEFT;
        } else {
            s.action = RIGHT;
        }
    }
}
