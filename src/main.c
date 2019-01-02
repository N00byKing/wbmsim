#include "../lib/lib.h"

#include <math.h>

#define PI 3.14159265358979

static struct {
    enum {
        STATUS_ANIMATING,
        STATUS_WAITING
    } status;
    enum {
        WIRE_BENT_UP,
        WIRE_BENT_DOWN,
        WIRE_STRAIGHT,
        WIRE_NONE
    } wire;
    enum {
        ACTION_BEND_UP,
        ACTION_BEND_DOWN,
        ACTION_ROLL_RIGHT,
        ACTION_ROLL_LEFT,
    } action;
    double animationStart, animationDuration, zoom;
    size_t circleQuality;
    uint8_t white[3], grey[3];
} s = {
    .status = STATUS_WAITING,
    .wire = WIRE_BENT_DOWN,
    .animationDuration = 1,
    .zoom = 0.5,
    .circleQuality = 40,
    .white = {255,255,255},
    .grey = {128,128,128}
};

int main(void) {
    Batch b = batchNew();
    xInit("Wire Bending Machine Simulator");

    while (!xOver()) {
        xPipe(s.zoom / xAR(), s.zoom, 0, 0);
        batchCircle(&b, 0,  1, 0.5, s.circleQuality, s.grey);
        batchCircle(&b, 0, -1, 0.5, s.circleQuality, s.grey);

        if (s.status == STATUS_WAITING) {
            switch (s.wire) {
                case WIRE_BENT_UP:
                    batchRingSlice(&b, 0, 1, 1, 1,-PI/2, PI/2, s.circleQuality/4, s.white);
                    break;
                case WIRE_BENT_DOWN:
                    batchRingSlice(&b, 0,-1, 1, 1, PI/2,-PI/2, s.circleQuality/4, s.white);
                    break;
                case WIRE_STRAIGHT:
                    batchLine(&b, 0, 0, 0, PI/2, 1, s.white);
                    break;
                case WIRE_NONE:
                    break;
            }
        } else {
        }

        batchDraw(&b);
        batchClear(&b);
        xSwap(0, 0, 0);

        if (xKey(XKEY_UP)) {
            s.wire = WIRE_BENT_UP;
        } else if (xKey(XKEY_DOWN)) {
            s.wire = WIRE_BENT_DOWN;
        } else if (xKey(XKEY_LEFT)) {
            s.wire = WIRE_NONE;
        } else if (xKey(XKEY_RIGHT)) {
            s.wire = WIRE_STRAIGHT;
        }
    }

    xExit();
    batchDel(&b);
}



//        float m[9], v[3] = {0, -2, 1}, mv[3];
//        float v2[3] = {0, -1, 1}, mv2[3];
//        matRot(m, (sinf(xTime()) + 1) * PI / 4);
//        matMulVec(mv, m, v);
//        matMulVec(mv2, m, v2);
//        mv[1] += 1;
//        mv2[1] += 1;
//
//        batchCircle(&b, 0,  1, 0.5, 40, (uint8_t[]){128,128,128});
//        batchCircle(&b, mv[0], mv[1], 0.5, 40, (uint8_t[]){128,128,128});
//        batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, (sinf(xTime()) + 1) * PI / 4, 40, (uint8_t[]){255,255,255});
//        batchLine(&b, mv2[0], mv2[1], (sinf(xTime()) + 1) * PI / 4, PI/2-(sinf(xTime())+1)*PI/4, 1, (uint8_t[]){255,255,255});
//        batchLine(&b, -xAR() * 2, 0, 0, xAR() * 2, 1, (uint8_t[]){255,255,255});
//
