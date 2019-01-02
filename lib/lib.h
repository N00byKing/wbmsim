#include <stddef.h>
#include <stdint.h>

#define XKEY_SPACE          32
#define XKEY_APOSTROPHE     39
#define XKEY_COMMA          44
#define XKEY_MINUS          45
#define XKEY_PERIOD         46
#define XKEY_SLASH          47
#define XKEY_0              48
#define XKEY_1              49
#define XKEY_2              50
#define XKEY_3              51
#define XKEY_4              52
#define XKEY_5              53
#define XKEY_6              54
#define XKEY_7              55
#define XKEY_8              56
#define XKEY_9              57
#define XKEY_SEMICOLON      59
#define XKEY_EQUAL          61
#define XKEY_A              65
#define XKEY_B              66
#define XKEY_C              67
#define XKEY_D              68
#define XKEY_E              69
#define XKEY_F              70
#define XKEY_G              71
#define XKEY_H              72
#define XKEY_I              73
#define XKEY_J              74
#define XKEY_K              75
#define XKEY_L              76
#define XKEY_M              77
#define XKEY_N              78
#define XKEY_O              79
#define XKEY_P              80
#define XKEY_Q              81
#define XKEY_R              82
#define XKEY_S              83
#define XKEY_T              84
#define XKEY_U              85
#define XKEY_V              86
#define XKEY_W              87
#define XKEY_X              88
#define XKEY_Y              89
#define XKEY_Z              90
#define XKEY_LEFT_BRACKET   91
#define XKEY_BACKSLASH      92
#define XKEY_RIGHT_BRACKET  93
#define XKEY_GRAVE_ACCENT   96
#define XKEY_ESCAPE        256
#define XKEY_ENTER         257
#define XKEY_TAB           258
#define XKEY_BACKSPACE     259
#define XKEY_INSERT        260
#define XKEY_DELETE        261
#define XKEY_RIGHT         262
#define XKEY_LEFT          263
#define XKEY_DOWN          264
#define XKEY_UP            265
#define XKEY_PAGE_UP       266
#define XKEY_PAGE_DOWN     267
#define XKEY_HOME          268
#define XKEY_END           269
#define XKEY_CAPS_LOCK     280
#define XKEY_SCROLL_LOCK   281
#define XKEY_NUM_LOCK      282
#define XKEY_PRINT_SCREEN  283
#define XKEY_PAUSE         284
#define XKEY_F1            290
#define XKEY_F2            291
#define XKEY_F3            292
#define XKEY_F4            293
#define XKEY_F5            294
#define XKEY_F6            295
#define XKEY_F7            296
#define XKEY_F8            297
#define XKEY_F9            298
#define XKEY_F10           299
#define XKEY_F11           300
#define XKEY_F12           301
#define XKEY_KP_0          320
#define XKEY_KP_1          321
#define XKEY_KP_2          322
#define XKEY_KP_3          323
#define XKEY_KP_4          324
#define XKEY_KP_5          325
#define XKEY_KP_6          326
#define XKEY_KP_7          327
#define XKEY_KP_8          328
#define XKEY_KP_9          329
#define XKEY_KP_DECIMAL    330
#define XKEY_KP_DIVIDE     331
#define XKEY_KP_MULTIPLY   332
#define XKEY_KP_SUBTRACT   333
#define XKEY_KP_ADD        334
#define XKEY_KP_ENTER      335
#define XKEY_KP_EQUAL      336
#define XKEY_LEFT_SHIFT    340
#define XKEY_LEFT_CONTROL  341
#define XKEY_LEFT_ALT      342
#define XKEY_LEFT_SUPER    343
#define XKEY_RIGHT_SHIFT   344
#define XKEY_RIGHT_CONTROL 345
#define XKEY_RIGHT_ALT     346
#define XKEY_RIGHT_SUPER   347
typedef struct {
    float x, y;
    uint8_t r, g, b;
} XVertex;
void xInit(const char *title);
void xExit(void);
float xAR(void);
void xPipe(float mulX, float mulY, float addX, float addY);
void xTris(size_t ni, const uint32_t *i, const XVertex *v);
void xSwap(uint8_t r, uint8_t g, uint8_t b);
int xOver(void);
double xTime(void);
int xKey(int key);

typedef struct {
    size_t ni, mi, nv, mv;
    uint32_t *i;
    XVertex *v;
} Batch;
Batch batchNew(void);
void batchDel(Batch *b);
void batchClear(Batch *b);
void batchDraw(const Batch *b);
void batchAny(Batch*b,size_t ni,const uint32_t*i,size_t nv,const XVertex*v);
void batchRect(Batch *b, const float *xywh, const uint8_t*rgb);
void batchRectLine(Batch*b,const float*xywh,float ti,float to,const uint8_t*rgb);
void batchLine(Batch*b,float x,float y,float a,float l,float t,const uint8_t*rgb);
void batchCircle(Batch*b,float x,float y,float r,size_t n,const uint8_t*rgb);
void batchPieSlice(Batch*b,float x,float y,float r,float o,float a,size_t n,const uint8_t*rgb);
void batchRing(Batch*b,float x,float y,float r,float t,size_t n,const uint8_t*rgb);
void batchRingSlice(Batch*b,float x,float y,float r,float t,float o,float a,size_t n,const uint8_t*rgb);

void matScl(float *m, float x, float y);
void matTrans(float *m, float x, float y);
void matRot(float *m, float a);
void matMul(float *m, const float *a, const float *b);
void matMulVec(float *mv, const float *m, const float *v);
