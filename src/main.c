#include "../lib/lib.h"

#include <math.h>

#define PI 3.14159265358979

#define MIN(x,y) (x)<(y)?(x):(y)
#define MAX(x,y) (x)>(y)?(x):(y)
#define CLAMP(min,val,max)  (MIN(max,MAX(min,val)))

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

        batchLine(&b, -xAR() / s.zoom, 0, 0, xAR() / s.zoom, 1, s.white);

        if (s.status == STATUS_ANIMATING) {
            float progress = (xTime() - s.animationStart) / s.animationDuration;
            progress = progress < 0 ? 0 : progress > 1 ? 1 : progress;
            if (progress >= 1) {
                s.status = STATUS_WAITING;
                switch (s.action) {
                    case ACTION_BEND_UP:
                        s.wire = s.wire != WIRE_NONE ? WIRE_BENT_UP : s.wire;
                        break;
                    case ACTION_BEND_DOWN:
                        s.wire = s.wire != WIRE_NONE ? WIRE_BENT_DOWN : s.wire;
                        break;
                    case ACTION_ROLL_RIGHT:
                        s.wire = WIRE_STRAIGHT;
                        break;
                    case ACTION_ROLL_LEFT:
                        s.wire = WIRE_NONE;
                        break;
                }
            } else {
                float m[9], v1[3] = {0, -2, 1}, v2[3] = {0, 2, 1}, v3[3] = {0, -1, 1}, v4[3] = {0, 1, 1}, mv1[3], mv2[3], mv3[3], mv4[3];
                matRot(m,  (1 - fabsf(progress * 2 - 1)) * PI / 2);
                matMulVec(mv1, m, v1);
                matMulVec(mv3, m, v3);
                matRot(m,  (1 - fabsf(progress * 2 - 1)) * -PI / 2);
                matMulVec(mv2, m, v2);
                matMulVec(mv4, m, v4);
                mv1[1] += 1;
                mv3[1] += 1;
                mv2[1] -= 1;
                mv4[1] -= 1;
                switch (s.action) {

                    case ACTION_BEND_UP:
                        switch (s.wire) {
                            case WIRE_BENT_UP:
                                batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, PI / 2, s.circleQuality, s.white);
                                break;
                            case WIRE_BENT_DOWN:
                                batchRingSlice(&b, mv1[0], mv1[1], 1, 1, PI/2+CLAMP(0, progress * 2, 1)*PI/2, -PI/2+CLAMP(0, progress * 2, 1)*PI/2, s.circleQuality, s.white);
                                batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, CLAMP(0, progress * 2, 1) * PI / 2, s.circleQuality, s.white);
                                break;
                            case WIRE_STRAIGHT:
                                batchRingSlice(&b, 0, 1, 1, 1, -PI / 2, CLAMP(0, progress * 2, 1) * PI / 2, s.circleQuality, s.white);
                                batchLine(&b, mv3[0], mv3[1], CLAMP(0, progress * 2, 1) *  PI / 2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, 1, s.white);
                                break;
                            case WIRE_NONE:
                                break;
                        }
                        batchCircle(&b, 0,  1, 0.5, s.circleQuality, s.grey);
                        batchCircle(&b, mv1[0], mv1[1], 0.5, s.circleQuality, s.grey);
                        break;

                    case ACTION_BEND_DOWN:
                        switch (s.wire) {
                            case WIRE_BENT_UP:
                                batchRingSlice(&b, mv2[0], mv2[1], 1, 1, -PI/2-CLAMP(0, progress * 2, 1)*PI/2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, s.circleQuality, s.white);
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2, CLAMP(0, progress * 2, 1) * -PI / 2, s.circleQuality, s.white);
                                // TODO 1
                                break;
                            case WIRE_BENT_DOWN:
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2,-PI/2, s.circleQuality, s.white);
                                break;
                            case WIRE_STRAIGHT:
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2, CLAMP(0, progress * 2, 1) * -PI / 2, s.circleQuality, s.white);
                                batchLine(&b, mv4[0], mv4[1], CLAMP(0, progress * 2, 1) * -PI / 2, PI/2-CLAMP(0, progress * 2, 1)*PI/2, 1, s.white);
                                break;
                            case WIRE_NONE:
                                break;
                        }
                        batchCircle(&b, mv2[0], mv2[1], 0.5, s.circleQuality, s.grey);
                        batchCircle(&b, 0, -1, 0.5, s.circleQuality, s.grey);
                        break;

                    case ACTION_ROLL_RIGHT:
                        batchLine(&b, 0, 0, 0, progress * PI / 2, 1, s.white);
                        batchCircle(&b, 0,  1, 0.5, s.circleQuality, s.grey);
                        batchCircle(&b, 0, -1, 0.5, s.circleQuality, s.grey);
                        break;

                    case ACTION_ROLL_LEFT:
                        switch (s.wire) {
                            case WIRE_BENT_UP:
                                batchRingSlice(&b, 0, 1, 1, 1,-PI/2, (1-progress)*PI/2, s.circleQuality, s.white);
                                break;
                            case WIRE_BENT_DOWN:
                                batchRingSlice(&b, 0,-1, 1, 1, PI/2,(1-progress)*-PI/2, s.circleQuality, s.white);
                                break;
                            case WIRE_STRAIGHT:
                                batchLine(&b, 0, 0, 0, PI / 2 - progress * PI / 2, 1, s.white);
                                break;
                            case WIRE_NONE:
                                break;
                        }
                        batchCircle(&b, 0,  1, 0.5, s.circleQuality, s.grey);
                        batchCircle(&b, 0, -1, 0.5, s.circleQuality, s.grey);
                        break;
                }
            }
        }

        if (s.status == STATUS_WAITING) {
            switch (s.wire) {
                case WIRE_BENT_UP:
                    batchRingSlice(&b, 0, 1, 1, 1,-PI/2, PI/2, s.circleQuality, s.white);
                    break;
                case WIRE_BENT_DOWN:
                    batchRingSlice(&b, 0,-1, 1, 1, PI/2,-PI/2, s.circleQuality, s.white);
                    break;
                case WIRE_STRAIGHT:
                    batchLine(&b, 0, 0, 0, PI/2, 1, s.white);
                    break;
                case WIRE_NONE:
                    break;
            }
            batchCircle(&b, 0,  1, 0.5, s.circleQuality, s.grey);
            batchCircle(&b, 0, -1, 0.5, s.circleQuality, s.grey);
        }

        batchDraw(&b);
        batchClear(&b);
        xSwap(0, 0, 0);

        if (s.status == STATUS_WAITING) {
            if (xKey(XKEY_UP) || xKey(XKEY_DOWN) || xKey(XKEY_LEFT) || xKey(XKEY_RIGHT)) {
                s.status = STATUS_ANIMATING;
                s.animationStart = xTime();
            }
            if (xKey(XKEY_UP)) {
                s.action = ACTION_BEND_UP;
            } else if (xKey(XKEY_DOWN)) {
                s.action = ACTION_BEND_DOWN;
            } else if (xKey(XKEY_LEFT)) {
                s.action = ACTION_ROLL_LEFT;
            } else if (xKey(XKEY_RIGHT)) {
                s.action = ACTION_ROLL_RIGHT;
            }
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
