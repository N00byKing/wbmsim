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
#define ZOOM 0.5
#define CIRCLE_QUALITY 40
#define PI 3.14159265358979
enum {UP, DOWN, LEFT, RIGHT};

static struct {
    int animating, wire, action;
    double animationStart;
} s = {.animating = 0, .wire = DOWN};

static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa);
static void stopAnimation(void);
static void batchStatic(Batch *b);

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

        batchLine(&b, -ar / ZOOM, 0, 0, ar / ZOOM, 1, WIRE_COLOR);

        if (s.animating) {
            float progress = (glfwGetTime() - s.animationStart) / ANIMATION_DURATION;
            progress = progress < 0 ? 0 : progress > 1 ? 1 : progress;
            if (progress >= 1) {
                stopAnimation();
            } else {
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
                switch (s.action) {

                    case UP:
                        switch (s.wire) {
                            case UP:
                                batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                                break;
                            case DOWN:
                                batchRingSlice(&b, mv1[0], mv1[1], 1, 1, PI/2+CLAMP(0, progress * 2, 1)*PI/2, -PI/2+CLAMP(0, progress * 2, 1)*PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                                batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, CLAMP(0, progress * 2, 1) * PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                                break;
                            case RIGHT:
                                batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, CLAMP(0, progress * 2, 1) * PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                                batchLine(&b, mv3[0], mv3[1], CLAMP(0, progress * 2, 1) *  PI / 2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, 1, WIRE_COLOR);
                                break;
                            case LEFT:
                                break;
                        }
                        batchCircle(&b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        batchCircle(&b, mv1[0], mv1[1], 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        break;

                    case DOWN:
                        switch (s.wire) {
                            case UP:
                                batchRingSlice(&b, mv2[0], mv2[1], 1, 1, -PI/2-CLAMP(0, progress * 2, 1)*PI/2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2, CLAMP(0, progress * 2, 1) * -PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                                break;
                            case DOWN:
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2,-PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                                break;
                            case RIGHT:
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2, CLAMP(0, progress * 2, 1) * -PI / 2, CIRCLE_QUALITY, WIRE_COLOR);
                                batchLine(&b, mv4[0], mv4[1], CLAMP(0, progress * 2, 1) * -PI / 2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, 1, WIRE_COLOR);
                                break;
                            case LEFT:
                                break;
                        }
                        batchCircle(&b, mv2[0], mv2[1], 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        batchCircle(&b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        break;

                    case RIGHT:
                        batchLine(&b, 0, 0, 0, progress * PI / 2, 1, WIRE_COLOR);
                        batchCircle(&b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        batchCircle(&b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        break;

                    case LEFT:
                        switch (s.wire) {
                            case UP:
                                batchRingSlice(&b, 0, 1, 1, 1,-PI/2, (1-progress)*PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                                break;
                            case DOWN:
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2,(1-progress)*-PI/2, CIRCLE_QUALITY, WIRE_COLOR);
                                break;
                            case RIGHT:
                                batchLine(&b, 0, 0, 0, PI / 2 - progress * PI / 2, 1, WIRE_COLOR);
                                break;
                            case LEFT:
                                break;
                        }
                        batchCircle(&b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        batchCircle(&b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
                        break;
                }
            }
        }

        if (!s.animating) {
            batchStatic(&b);
        }

        batchDraw(&b);
        batchClear(&b);

        glfwSwapBuffers(win);

        if (!s.animating) {
            if (glfwGetKey(win, GLFW_KEY_UP) || glfwGetKey(win, GLFW_KEY_DOWN) || glfwGetKey(win, GLFW_KEY_LEFT) || glfwGetKey(win, GLFW_KEY_RIGHT)) {
                s.animating = 1;
                s.animationStart = glfwGetTime();
            }
            if (glfwGetKey(win, GLFW_KEY_UP)) {
                s.action = UP;
            } else if (glfwGetKey(win, GLFW_KEY_DOWN)) {
                s.action = DOWN;
            } else if (glfwGetKey(win, GLFW_KEY_LEFT)) {
                s.action = LEFT;
            } else if (glfwGetKey(win, GLFW_KEY_RIGHT)) {
                s.action = RIGHT;
            }
        }
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

static void stopAnimation(void) {
    s.animating = 0;
    if (s.action == UP && s.wire != LEFT) {
        s.wire = UP;
    } else if (s.action == DOWN && s.wire != LEFT) {
        s.wire = DOWN;
    } else if (s.action == RIGHT) {
        s.wire = RIGHT;
    } else if (s.action == LEFT) {
        s.wire = LEFT;
    }
}

static void batchStatic(Batch *b) {
    if (s.wire == UP) {
        batchRingSlice(b, 0, 1, 1, 1,-PI/2, PI/2, CIRCLE_QUALITY, WIRE_COLOR);
    } else if (s.wire == DOWN) {
        batchRingSlice(b, 0,-1, 1, 1, PI/2,-PI/2, CIRCLE_QUALITY, WIRE_COLOR);
    } else if (s.wire == RIGHT) {
        batchLine(b, 0, 0, 0, PI/2, 1, WIRE_COLOR);
    }
    batchCircle(b, 0,  1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
    batchCircle(b, 0, -1, 0.5, CIRCLE_QUALITY, CIRCLE_COLOR);
}
