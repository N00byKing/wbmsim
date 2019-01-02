#include "../lib/lib.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <math.h>
#define PI 3.14159265358979

int main(void) {
    Batch b = batchNew();

    xInit("Wire Bending Machine Simulator");
    xPipe(1.0/xAR()/2, 1.0/2, 0, 0);
    while (!xOver()) {

        float m[9], v[3] = {0, -2, 1}, mv[3];
        float v2[3] = {0, -1, 1}, mv2[3];
        matRot(m, (sinf(glfwGetTime()) + 1) * PI / 4);
        matMulVec(mv, m, v);
        matMulVec(mv2, m, v2);
        mv[1] += 1;
        mv2[1] += 1;

        batchCircle(&b, 0,  1, 0.5, 40, (uint8_t[]){128,128,128});
        batchCircle(&b, mv[0], mv[1], 0.5, 40, (uint8_t[]){128,128,128});
        batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, (sinf(glfwGetTime()) + 1) * PI / 4, 40, (uint8_t[]){255,255,255});
        batchLine(&b, mv2[0], mv2[1], (sinf(glfwGetTime()) + 1) * PI / 4, PI/2-(sinf(glfwGetTime())+1)*PI/4, 1, (uint8_t[]){255,255,255});
        batchLine(&b, -xAR() * 2, 0, 0, xAR() * 2, 1, (uint8_t[]){255,255,255});

        batchDraw(&b);
        batchClear(&b);
        xSwap(0, 0, 0);
    }
    xExit();

    batchDel(&b);
}
