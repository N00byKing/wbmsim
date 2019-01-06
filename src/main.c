#include "../lib/lib.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <math.h>

#define MIN(x,y) ((x)<(y)?(x):(y))

#define WIN_T "Wire Bending Machine Simulator"
#define OGL_API GLFW_OPENGL_ES_API
#define OGL_V 20
#define VSYNC 1
#define MSAA 16
#define CLRL (uint8_t[]){255,255,255}
#define CLRC  (uint8_t[]){128,128,128}
#define DT 2
#define ZOOM 0.25
#define Q 40
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
static void batchAnimatedCircles(Batch *b, float progress);
static void batchAnimatedActiveWire(Batch *b, float progress);
static void batchAnimatedInactiveWire(Batch *b, float progress);
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
            float progress = (glfwGetTime() - s.animationStart) / DT;
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

    if (s.action == UP) {
        if (wire == RIGHT) {
            matTrans(m0, -PI/2, 0);
            matRot(m1, PI/2);
            matMul(m2, m1, m0);
            matTrans(m1, 1, 1);
            matMul(m0, m1, m2);
        } else if (wire == DOWN) {
            matTrans(m0, -1, 1);
            matRot(m1, PI);
            matMul(m2, m1, m0);
            matTrans(m1, 1, 1);
            matMul(m0, m1, m2);
        } else {
            return;
        }
    } else if (s.action == DOWN) {
        if (wire == RIGHT) {
            matTrans(m0, -PI/2, 0);
            matRot(m1, -PI/2);
            matMul(m2, m1, m0);
            matTrans(m1, 1, -1);
            matMul(m0, m1, m2);
        } else if (wire == UP) {
            matTrans(m0, -1, -1);
            matRot(m1, -PI);
            matMul(m2, m1, m0);
            matTrans(m1, 1, -1);
            matMul(m0, m1, m2);
        } else {
            return;
        }
    } else if (s.action == LEFT) {
        if (s.wireLen <= 1) {
            return;
        } else if (s.wire[s.wireLen - 2] == RIGHT) {
            batchClearLine(&s.b);
        } else {
            batchClearRingSlice(&s.b, Q);
        }
        if (wire == RIGHT) {
            matTrans(m0, -PI/2, 0);
        } else if (wire == UP) {
            matTrans(m0, 1, -1);
            matRot(m1, -PI/2);
            matMul(m2, m1, m0);
            matTrans(m1, 0, 2);
            matMul(m0, m1, m2);
        } else {
            matTrans(m0, 1, 1);
            matRot(m1, PI/2);
            matMul(m2, m1, m0);
            matTrans(m1, 0, -2);
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
    batchLine(b, -ar / ZOOM, 0, 0, ar / ZOOM, 1, CLRL);
    batchStaticActiveWire(b);
    batchCircle(b, 0,  1, 0.5, Q, CLRC);
    batchCircle(b, 0, -1, 0.5, Q, CLRC);
    batchDraw(&s.b);
}

static void batchStaticActiveWire(Batch *b) {
    if (s.wireLen > 0 && s.wire[s.wireLen - 1] == UP) {
        batchRingSlice(b, 0, 1, 1, 1,-PI/2, PI/2, Q, CLRL);
    } else if (s.wireLen > 0 && s.wire[s.wireLen - 1] == DOWN) {
        batchRingSlice(b, 0,-1, 1, 1, PI/2,-PI/2, Q, CLRL);
    } else if (s.wireLen > 0 && s.wire[s.wireLen - 1] == RIGHT) {
        batchLine(b, 0, 0, 0, PI/2, 1, CLRL);
    }
}

static void batchAnimated(Batch *b, float progress, float ar) {
    batchLine(b, -ar / ZOOM, 0, 0, ar / ZOOM, 1, CLRL);
    batchAnimatedActiveWire(b, progress);
    batchAnimatedInactiveWire(b, progress);
    batchAnimatedCircles(b, progress);
}

static void batchAnimatedCircles(Batch *b, float progress) {
    float m[9], mv1[3], mv2[3];
    float v1[] = {0, -2, 1};
    float v2[] = {0,  2, 1};

    matRot(m,  (1 - fabsf(progress * 2 - 1)) *  PI/2);
    matMulVec(mv1, m, v1);
    matRot(m,  (1 - fabsf(progress * 2 - 1)) * -PI/2);
    matMulVec(mv2, m, v2);
    mv1[1] += 1;
    mv2[1] -= 1;

    if (s.action == UP) {
        batchCircle(b, 0,  1, 0.5, Q, CLRC);
        batchCircle(b, mv1[0], mv1[1], 0.5, Q, CLRC);
    } else if (s.action == DOWN) {
        batchCircle(b, mv2[0], mv2[1], 0.5, Q, CLRC);
        batchCircle(b, 0, -1, 0.5, Q, CLRC);
    } else if (s.action == RIGHT) {
        batchCircle(b, 0,  1, 0.5, Q, CLRC);
        batchCircle(b, 0, -1, 0.5, Q, CLRC);
    } else {
        batchCircle(b, 0,  1, 0.5, Q, CLRC);
        batchCircle(b, 0, -1, 0.5, Q, CLRC);
    }
}

static void batchAnimatedActiveWire(Batch *b, float progress) {
    float m[9], mv1[3], mv2[3], mv3[3], mv4[3];
    float v1[] = {0, -2, 1};
    float v2[] = {0,  2, 1};
    float v3[] = {0, -1, 1};
    float v4[] = {0,  1, 1};

    matRot(m,  (1 - fabsf(progress * 2 - 1)) *  PI/2);
    matMulVec(mv1, m, v1);
    matMulVec(mv3, m, v3);
    matRot(m,  (1 - fabsf(progress * 2 - 1)) * -PI/2);
    matMulVec(mv2, m, v2);
    matMulVec(mv4, m, v4);
    mv1[1] += 1;
    mv3[1] += 1;
    mv2[1] -= 1;
    mv4[1] -= 1;

    int wire = s.wireLen > 0 ? s.wire[s.wireLen - 1] : LEFT;
    float x = MIN(progress * 2, 1) * PI/2;

    if (s.action == UP) {
        if (wire == UP) {
            batchRingSlice(b, 0, 1, 1, 1, -PI/2, PI/2, Q, CLRL);
        } else if (wire == DOWN) {
            batchRingSlice(b, mv1[0], mv1[1], 1, 1, PI/2+x, -PI/2+x, Q, CLRL);
            batchRingSlice(b, 0, 1, 1, 1, -PI/2, x, Q, CLRL);
        } else if (wire == RIGHT) {
            batchRingSlice(b, 0, 1, 1, 1, -PI/2, x, Q, CLRL);
            batchLine(b, mv3[0], mv3[1], x, PI/2-x, 1, CLRL);
        }
    } else if (s.action == DOWN) {
        if (wire == UP) {
            batchRingSlice(b, mv2[0], mv2[1], 1, 1, -PI/2-x, PI/2-x, Q, CLRL);
            batchRingSlice(b, 0,-1, 1, 1, PI/2, -x, Q, CLRL);
        } else if (wire == DOWN) {
            batchRingSlice(b, 0,-1, 1, 1, PI/2,-PI/2, Q, CLRL);
        } else if (wire == RIGHT) {
            batchRingSlice(b, 0,-1, 1, 1, PI/2, -x, Q, CLRL);
            batchLine(b, mv4[0], mv4[1], -x, PI/2-x, 1, CLRL);
        }
    } else if (s.action == LEFT) {
        if (wire == UP) {
            batchRingSlice(b, 0, 1, 1, 1,-PI/2, (1-progress)*PI/2, Q, CLRL);
        } else if (wire == DOWN) {
            batchRingSlice(b, 0,-1, 1, 1, PI/2,(1-progress)*-PI/2, Q, CLRL);
        } else if (wire == RIGHT) {
            batchLine(b, 0, 0, 0, PI/2 - progress * PI/2, 1, CLRL);
        }
    } else {
        batchLine(b, 0, 0, 0, progress * PI/2, 1, CLRL);
    }
}

static void batchAnimatedInactiveWire(Batch *b, float progress) {
    float m0[9], m1[9], m2[9], mv1[3], mv2[3], mv3[3], mv4[3], mv5[3], mv6[3], mv7[3], mv8[3], mv9[3], mv0[3];;
    int wire = s.wireLen < 1 ? LEFT : s.wire[s.wireLen - 1];
    if (wire == LEFT) {
        return;
    }

    float v1[] = {PI/2 * (1 - MIN(progress * 2, 1)), 0, 1};
    float v2[] = {0, -1, 1};
    float v3[] = {PI/2 * (1 - MIN(progress * 2, 1)), 0, 1};
    float v4[] = {0,  1, 1};
    float v5[] = {0, -2, 1};
    float v6[] = {1,  0, 1};
    float v7[] = {0,  2, 1};
    float v8[] = {1, 0, 1};
    float v9[] = {1, 0, 0};

    matRot(m0, MIN(progress * 2, 1) *  PI/2);
    matMulVec(mv1, m0, v1);
    matMulVec(mv2, m0, v2);
    matRot(m0, MIN(progress * 2, 1) * -PI/2);
    matMulVec(mv3, m0, v3);
    matMulVec(mv4, m0, v4);
    matRot(m0, MIN(progress * 2, 1) *  PI/2);
    matMulVec(mv5, m0, v5);
    matRot(m0, MIN(progress * 2, 1) *  PI);
    matMulVec(mv6, m0, v6);
    matRot(m0, MIN(progress * 2, 1) *  -PI/2);
    matMulVec(mv7, m0, v7);
    matRot(m0, MIN(progress * 2, 1) *  -PI);
    matMulVec(mv8, m0, v8);
    matRot(m0, progress * -PI/2);
    matMulVec(mv9, m0, v9);
    matRot(m0, progress * PI/2);
    matMulVec(mv0, m0, v9);
    mv2[1] += 1;
    mv4[1] -= 1;
    mv5[1] += 1;
    mv7[1] -= 1;
    mv9[1] += 1;
    mv0[1] -= 1;

    batchAny(b, s.b.ni, NULL, s.b.nv, NULL);

    if (s.action == UP) {
        if (wire == RIGHT) {
            matTrans(m0, -PI/2, 0);
            matRot(m1, PI/2 * MIN(progress * 2, 1));
            matMul(m2, m1, m0);
            matTrans(m1, mv1[0] + mv2[0], mv1[1] + mv2[1]);
            matMul(m0, m1, m2);
        } else if (wire == DOWN) {
            matTrans(m0, -1, 1);
            matRot(m1, PI * MIN(progress * 2, 1));
            matMul(m2, m1, m0);
            matTrans(m1, mv5[0] + mv6[0], mv5[1] + mv6[1]);
            matMul(m0, m1, m2);
        } else {
            matTrans(m0, 0, 0);
        }
    } else if (s.action == DOWN) {
        if (wire == RIGHT) {
            matTrans(m0, -PI/2, 0);
            matRot(m1, -PI/2 * MIN(progress * 2, 1));
            matMul(m2, m1, m0);
            matTrans(m1, mv3[0] + mv4[0], mv3[1] + mv4[1]);
            matMul(m0, m1, m2);
        } else if (wire == UP) {
            matTrans(m0, -1, -1);
            matRot(m1, -PI * MIN(progress * 2, 1));
            matMul(m2, m1, m0);
            matTrans(m1, mv7[0] + mv8[0], mv7[1] + mv8[1]);
            matMul(m0, m1, m2);
        } else {
            matTrans(m0, 0, 0);
        }
    } else if (s.action == LEFT) {
        if (wire == UP) {
            matTrans(m0, -1, -1);
            matRot(m1, -PI/2 * progress);
            matMul(m2, m1, m0);
            matTrans(m1, mv9[0], mv9[1]);
            matMul(m0, m1, m2);
        } else if (wire == DOWN) {
            matTrans(m0, -1, 1);
            matRot(m1, PI/2 * progress);
            matMul(m2, m1, m0);
            matTrans(m1, mv0[0], mv0[1]);
            matMul(m0, m1, m2);
        } else {
            matTrans(m0, -progress * PI/2, 0);
        }
    } else {
        if (s.wire[s.wireLen - 1] == UP) {
            batchRingSlice(b, progress * PI/2, 1, 1, 1,-PI/2, PI/2, Q, CLRL);
        } else if (s.wire[s.wireLen - 1] == DOWN) {
            batchRingSlice(b, progress * PI/2,-1, 1, 1, PI/2,-PI/2, Q, CLRL);
        } else {
            batchLine(b, progress * PI/2, 0, 0, PI/2, 1, CLRL);
        }
        matTrans(m0, progress * PI/2, 0);
    }

    for (size_t i = 0; i < s.b.ni; ++i) {
        b->i[b->ni + i] = b->nv + s.b.i[i];
    }
    b->ni += s.b.ni;

    for (size_t i = 0; i < s.b.nv; ++i) {
        float v[] = {s.b.v[i].x, s.b.v[i].y, 1};
        matMulVec(mv1, m0, v);
        b->v[b->nv + i].x = mv1[0];
        b->v[b->nv + i].y = mv1[1];
        b->v[b->nv + i].r = CLRL[0];
        b->v[b->nv + i].g = CLRL[1];
        b->v[b->nv + i].b = CLRL[2];
    }
    b->nv += s.b.nv;
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
