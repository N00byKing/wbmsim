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
    } animation;
} s;

bool isValidWire(const char *w);
