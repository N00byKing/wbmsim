#include <stdbool.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../lib/lib.h"
#include "main.h"

#define WIN_T "Wire Bending Machine Simulator"
#define OGL_API GLFW_OPENGL_ES_API
#define OGL_V 20
#define VSYNC 1
#define MSAA 16

static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa);

int main(void) {
    glfwInit();
    GLFWwindow *win = mkWin(WIN_T, OGL_API, OGL_V, VSYNC, MSAA);
    rInit();
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int winW, winH;
        glfwGetFramebufferSize(win, &winW, &winH);
        rViewport(0, 0, winW, winH);
        float ar = (float)winW / winH;
        rPipe(0.25 / ar, 0.25, 0, 0);

        rClear(0, 0, 0);
        glfwSwapBuffers(win);
    }
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
