#include "../lib/lib.h"

int main(void) {
    xInit("Wire Bending Machine Simulator");
    while (!xOver()) {
        xTris(3, (const uint32_t[]){0,1,2}, (const XTriV[]){{-1,-1,255,255,255},{0,1,255,255,255},{1,-1,255,255,255}});
        xSwap(0, 0, 0);
    }
    xExit();
}
