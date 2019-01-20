#include <stdbool.h>
#include <math.h>

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
#define CCT 0.05 // Circle Contour Thickness
#define CSR 0.05 // Circle Screw Radius
#define CSO 0.35 // Circle Screw Offset from circle
#define CSN 8 // Circle Screw Count
#define CSC (const uint8_t[]){64, 64, 64} // Circle Screw Color
#define WC (const uint8_t[]){255, 255, 255} // Wire Color
#define CC (const uint8_t[]){128, 128, 128} // Circle Color
#define CCC (const uint8_t[]){64, 64, 64} // Circle Contour Color

struct S s;

static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa);
static void draw(float ar);

int main(void) {
    glfwInit();
    GLFWwindow *win = mkWin(WIN_T, OGL_API, OGL_V, VSYNC, MSAA);
    rInit();
    s.b = batchNew();

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
    }

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
    batchRect(&s.b, (const float[]){-ar / ZOOM + CX, -WT / 2 + CY, ar / ZOOM, WT}, WC);

    float da = PI * 2 / CSN;

    batchCircle(&s.b, CX, CY + WT, WT / 2, QC, CC);
    batchRing(&s.b, CX, CY + WT, WT / 2 - CCT / 2, CCT, QC, CCC);
    for (size_t i = 0; i < CSN; ++i) {
        float x = cos(da * i) * CSO + CX;
        float y = sin(da * i) * CSO + CY + WT;
        batchCircle(&s.b, x, y, CSR, QCS, CSC);
    }

    batchCircle(&s.b, CX, CY - WT, WT / 2, QC, CC);
    batchRing(&s.b, CX, CY - WT, WT / 2 - CCT / 2, CCT, QC, CCC);
    for (size_t i = 0; i < CSN; ++i) {
        float x = cos(da * i) * CSO + CX;
        float y = sin(da * i) * CSO + CY - WT;
        batchCircle(&s.b, x, y, CSR, QCS, CSC);
    }

    rTris(s.b.ni, s.b.i, s.b.v);

    batchClear(&s.b);
}
