#include "../lib/lib.h"

int main(void) {
    xInit("Wire Bending Machine Simulator");
    while (!xOver()) {
        xSwap(0, 0, 0);
    }
    xExit();
}
