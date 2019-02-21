typedef struct {
    float x, y, w, h;
} WOpRect;
bool wOpIsValid(const char *w);
char *wOpNextW(const char *wire, char wActive, char action);
WOpRect wOpGetRect(const char *w);
