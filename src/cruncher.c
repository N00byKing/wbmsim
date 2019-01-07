#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static size_t s2n(const char *s);
static void n2w(size_t n, char *w, size_t L);
static size_t w2n(const char *w, size_t L);
static void reverse(char *w, size_t L);
static void rotate(char *w, size_t L);

int main(int argc, char **argv) {
    size_t L = s2n(argv[argc - 1]);
    size_t N = round(pow(3, L));
    size_t n = 0;
    char *p = calloc(N, 1);
    for (size_t i = 0; i < N; ++i) {
        if (p[i]) {
            continue;
        }
        p[i] = 1;
        char w[L];
        n2w(i, w, L);
        reverse(w, L);
        p[w2n(w, L)] = 1;
        rotate(w, L);
        p[w2n(w, L)] = 1;
        reverse(w, L);
        p[w2n(w, L)] = 1;
        ++n;
    }
    free(p);
    printf("%zu\n", n);
}

static size_t s2n(const char *s) {
    size_t n = 0;
    while (*s) {
        n *= 10;
        n += *s - '0';
        ++s;
    }
    return n;
}

static void n2w(size_t n, char *w, size_t L) {
    for (size_t j = 0; j < L; ++j) {
        w[j] = n % 3 == 2 ? 'D' : n % 3 == 1 ? 'U' : 'R';
        n /= 3;
    }
}

static size_t w2n(const char *w, size_t L) {
    size_t n = 0;
    for (size_t j = L - 1; j < L; --j) {
        n += (w[j] == 'D') ? 2 : (w[j] == 'U') ? 1 : 0;
        if (j > 0) {
            n *= 3;
        }
    }
    return n;
}

static void reverse(char *w, size_t L) {
    for (size_t i = 0; i < L / 2; ++i) {
        char t = w[i];
        w[i] = w[L - i - 1];
        w[L - i - 1] = t;
    }
}

static void rotate(char *w, size_t L) {
    for (size_t i = 0; i < L; ++i) {
        w[i] = (w[i] == 'U') ? 'D' : (w[i] == 'D') ? 'U' : 'R';
    }
}
