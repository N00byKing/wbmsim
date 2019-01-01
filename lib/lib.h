#include <stddef.h>
#include <stdint.h>

typedef struct {
    float x, y;
    uint8_t r, g, b;
} XTriV;
void xInit(const char *title);
void xExit(void);
float xAR(void);
void xPipe(float mulX, float mulY, float addX, float addY);
void xTris(size_t ni, const uint32_t *i, const XTriV *v);
void xSwap(uint8_t r, uint8_t g, uint8_t b);
int xOver(void);
