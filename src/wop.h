typedef struct {
    float x, y, w, h;
} WOpRect;
bool wOpIsValid(const char *w);
char *wOpCurrW(const char *wire, char wActive);
char *wOpNextW(const char *wire, char wActive, char action);
WOpRect wOpGetRect(const char *w0, const char *w1, bool animation, char action, float dt);
