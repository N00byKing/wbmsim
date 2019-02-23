#include <stddef.h>
#include <stdint.h>

typedef struct {
    float x, y;
    uint8_t r, g, b;
} RVertex;
void rInit(void);
void rExit(void);
void rPipe(float mulX, float mulY, float addX, float addY);
void rTris(size_t ni, const uint32_t *i, const RVertex *v);
void rClear(uint8_t r, uint8_t g, uint8_t b);
void rViewport(int x, int y, int w, int h);

typedef struct {
    size_t ni, mi, nv, mv;
    uint32_t *i;
    RVertex *v;
} Batch;
Batch batchNew(void);
void batchDel(Batch *b);
void batchClear(Batch *b);
void batchDraw(const Batch *b);
void batchAny(Batch*b,size_t ni,const uint32_t*i,size_t nv,const RVertex*v);
void batchRect(Batch *b, const float *xywh, const uint8_t*rgb);
void batchRectLine(Batch*b,const float*xywh,float ti,float to,const uint8_t*rgb);
void batchLine(Batch*b,float x,float y,float a,float l,float t,const uint8_t*rgb);
void batchCircle(Batch*b,float x,float y,float r,float o,size_t n,const uint8_t*rgb);
void batchPieSlice(Batch*b,float x,float y,float r,float o,float a,size_t n,const uint8_t*rgb);
void batchRing(Batch*b,float x,float y,float r,float t,float o,size_t n,const uint8_t*rgb);
void batchRingSlice(Batch*b,float x,float y,float r,float t,float o,float a,size_t n,const uint8_t*rgb);
void batchClearAny(Batch*b,size_t ni,size_t nv);
void batchClearRect(Batch *b);
void batchClearRectLine(Batch*b);
void batchClearLine(Batch*b);
void batchClearCircle(Batch*b,size_t n);
void batchClearPieSlice(Batch*b,size_t n);
void batchClearRing(Batch*b,size_t n);
void batchClearRingSlice(Batch*b,size_t n);

void matScl(float *m, float x, float y);
void matTrans(float *m, float x, float y);
void matRot(float *m, float a);
void matMul(float *m, const float *a, const float *b);
void matMulVec(float *mv, const float *m, const float *v);
