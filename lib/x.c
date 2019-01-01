#include "lib.h"

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

static struct {
    GLFWwindow *win;
} x;

static GLFWwindow *mkW(const char *t, int api, int v, int vs, int aa);

void xInit(const char *title) {
    glfwInit();
    x.win = mkW(title, GLFW_OPENGL_ES_API, 20, 1, 16);
    glClear(GL_COLOR_BUFFER_BIT);
    xSwap(0, 0, 0);
}

void xExit(void) {
    glfwTerminate();
}

float xAR(void) {
    int w, h;
    glfwGetFramebufferSize(x.win, &w, &h);
    return (float)w / (float)h;
}

void xSwap(uint8_t r, uint8_t g, uint8_t b) {
    glfwPollEvents();

    glfwSwapBuffers(x.win);

    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int w, h;
    glfwGetFramebufferSize(x.win, &w, &h);
    glViewport(0, 0, w, h);
}

int xOver(void) {
    return glfwWindowShouldClose(x.win);
}

static GLFWwindow *mkW(const char *t, int api, int v, int vs, int aa) {
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
