#include "string.h"

size_t strlen(const char *str) {
    const char *s = str;
    while (*s++);

    return s - str - 1;
}

char *strcpy(char *dst, const char *src) {
    char c;
    char *p = dst;

    while ((c = *src++)) {
        *p++ = c;
    }

    *p = '\0';
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    size_t i;

    for (i = 0 ; i < n && src[i] != '\0' ; i++) {
        dst[i] = src[i];
    }

    for ( ; i < n ; i++) {
        dst[i] = '\0';
    }

    return dst;
}


int strcmp(const char *s1, const char *s2) {
    while (*s1 == *s2) {
        if (*s1 == '\0') {
            return 0;
        }
        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}


char *strcpy_safe(char *dst, const char *src, size_t dstSize) {
    char *p = dst;
    char *end = dst + dstSize - 1;
    char c;

    while ((c = *src++) && dst < end) {
        *p++ = c;
    }
    if (p < end + 1) {
        *p = '\0';
    }

    return dst;
}