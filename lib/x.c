#include "lib.h"

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

static struct {
    GLFWwindow *win;
    GLuint prog, aPos, aClr, uMul, uAdd;
} x;

static GLFWwindow *mkWin(const char *t, int api, int v, int vs, int aa);
static GLuint mkShd(const char *vertSrc, const char *fragSrc);

void xInit(const char *title) {
    const char *VERT =
    "#version 100\n"
    "attribute vec2 aPos;\n"
    "attribute vec3 aClr;\n"
    "varying vec3 vClr;\n"
    "uniform vec2 uMul, uAdd;\n"
    "void main(void) {\n"
    "    gl_Position = vec4(aPos * uMul + uAdd, 0, 1);\n"
    "    vClr = aClr / 255.0;\n"
    "}\n";

    const char *FRAG =
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec3 vClr;\n"
    "void main(void) {\n"
    "    gl_FragColor = vec4(vClr.rgb, 1);\n"
    "}\n";

    glfwInit();
    x.win = mkWin(title, GLFW_OPENGL_ES_API, 20, 1, 16);

    x.prog = mkShd(VERT, FRAG);
    glUseProgram(x.prog);

    x.aPos = glGetAttribLocation(x.prog, "aPos");
    x.aClr = glGetAttribLocation(x.prog, "aClr");
    x.uMul = glGetUniformLocation(x.prog, "uMul");
    x.uAdd = glGetUniformLocation(x.prog, "uAdd");

    xPipe(1, 1, 0, 0);

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

void xPipe(float mulX, float mulY, float addX, float addY) {
    glUniform2f(x.uMul, mulX, mulY);
    glUniform2f(x.uAdd, addX, addY);
}

void xTris(size_t ni, const uint32_t *i, const XTriV *v) {
    glEnableVertexAttribArray(x.aPos);
    glEnableVertexAttribArray(x.aClr);

    glVertexAttribPointer(x.aPos, 2, GL_FLOAT, GL_FALSE, sizeof(*v), &v->x);
    glVertexAttribPointer(x.aClr,3,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(*v),&v->r);

    glDrawElements(GL_TRIANGLES, ni, GL_UNSIGNED_INT, i);

    glDisableVertexAttribArray(x.aClr);
    glDisableVertexAttribArray(x.aPos);
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

static GLuint mkShd(const char *vertSrc, const char *fragSrc) {
    GLuint prog = glCreateProgram();
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vert, 1, &vertSrc, NULL);
    glShaderSource(frag, 1, &fragSrc, NULL);
    glCompileShader(vert);
    glCompileShader(frag);
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDetachShader(prog, vert);
    glDetachShader(prog, frag);
    glDeleteShader(frag);
    glDeleteShader(vert);
    return prog;
}
