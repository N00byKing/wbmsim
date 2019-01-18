#include "lib.h"

#include <GLES2/gl2.h>

static struct {
    GLuint prog, aPos, aClr, uMul, uAdd;
} r;

static GLuint mkShd(const char *vertSrc, const char *fragSrc);

void rInit(void) {
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

    r.prog = mkShd(VERT, FRAG);
    glUseProgram(r.prog);

    r.aPos = glGetAttribLocation(r.prog, "aPos");
    r.aClr = glGetAttribLocation(r.prog, "aClr");
    r.uMul = glGetUniformLocation(r.prog, "uMul");
    r.uAdd = glGetUniformLocation(r.prog, "uAdd");

    rPipe(1, 1, 0, 0);
}

void rExit(void) {
    glDeleteProgram(r.prog);
}

void rPipe(float mulX, float mulY, float addX, float addY) {
    glUniform2f(r.uMul, mulX, mulY);
    glUniform2f(r.uAdd, addX, addY);
}

void rTris(size_t ni, const uint32_t *i, const float *v) {
    glEnableVertexAttribArray(r.aPos);
//    glEnableVertexAttribArray(r.aClr);

    glVertexAttribPointer(r.aPos, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, v);
//    glVertexAttrib3f(r.aClr, 1, 1, 1);
//    glVertexAttribPointer(r.aClr,3,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(*v),&v->r);

    glDrawElements(GL_LINES, ni, GL_UNSIGNED_INT, i);

//    glDisableVertexAttribArray(r.aClr);
    glDisableVertexAttribArray(r.aPos);
}

void rClear(uint8_t r, uint8_t g, uint8_t b) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void rViewport(int x, int y, int w, int h) {
    glViewport(x, y, w, h);
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
