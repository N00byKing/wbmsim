#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#define WIN_W 0
#define WIN_H 0
#define WIN_T "Wire Bending Machine Simulator"
#define OGL_API GLFW_OPENGL_ES_API
#define OGL_V 20
#define VSYNC 1
#define AA 4

static GLFWwindow*mkW(int w,int h,const char*t,int api,int v,int vs,int aa);

int main(void) {
    glfwInit();
    GLFWwindow *win = mkW(WIN_W, WIN_H, WIN_T, OGL_API, OGL_V, VSYNC, AA);
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(win);
    }
    glfwTerminate();
}

static GLFWwindow*mkW(int w,int h,const char*t,int api,int v,int vs,int aa) {
    GLFWwindow *win;
    int width = w, height = h;
    GLFWmonitor *monitor = NULL;

    glfwWindowHint(GLFW_CLIENT_API, api);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, v / 10);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, v % 10);
    glfwWindowHint(GLFW_SAMPLES, aa);

    if (w <= 0 || h <= 0) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
        width = vidmode->width;
        height = vidmode->height;
        glfwWindowHint(GLFW_RED_BITS, vidmode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, vidmode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, vidmode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, vidmode->refreshRate);
    }

    win = glfwCreateWindow(width, height, t, monitor, NULL);
    glfwMakeContextCurrent(win);
    glfwSwapInterval(vs);

    return win;
}
