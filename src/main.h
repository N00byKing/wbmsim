#define PI 3.1415926535
#define QQ 40 // Quality of Quarter rings
#define QC 40 // Quality of Circle
#define QCS 8 // Quality of Circle Screw

extern struct S {
    Batch b;
    struct {
        int on;
        double start;
        char action;
        double x, y, w, h;
    } animation;
    struct {
        size_t n, m;
        char active, *passive;
        double x, y, w, h;
    } wire;
} s;

// TODO: rename to wireOp*
bool isValidWire(const char *w);
void getWireSize(const char *wire, double *x, double *y, double *w, double *h);
