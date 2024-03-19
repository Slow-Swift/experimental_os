#include "string.h"

void *memcpy(void * restrict s1, const void * restrict s2, size_t n) {
    char *current1 = s1;
    const char *current2 = s2;
    for (int i = 0; i < n; i++, current1++, current2++) {
        *current1 = *current2;
    }
    return s1;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const char *current1 = s1;
    const char *current2 = s2;
    for (int i = 0; i < n; i++, current1++, current2++) {
        if (*current1 != *current2) {
            return (int)*current1 - (int)*current2;
        }
    }
    return 0;
}

