#include "../lib/lib.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <math.h>
#define PI 3.14159265358979

int main(void) {
    Batch b = batchNew();

    xInit("Wire Bending Machine Simulator");
    xPipe(1.0/xAR()/10, 1.0/10, 0, 0);
    while (!xOver()) {
        batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, PI / 2, 40, (uint8_t[]){255,255,255});
        batchCircle(&b, 0,  1, 0.5, 40, (uint8_t[]){128,128,128});
        batchCircle(&b, 0, -1, 0.5, 40, (uint8_t[]){128,128,128});
        batchDraw(&b);
        batchClear(&b);
        xSwap(0, 0, 0);
    }
    xExit();

    batchDel(&b);
}
