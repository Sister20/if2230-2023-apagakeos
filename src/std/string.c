#include "string.h"

void clear(void *pointer, size_t n) {
    uint8_t *ptr       = (uint8_t*) pointer;
    for (size_t i = 0; i < n; i++) {
        ptr[i] = 0x00;
    }
}

size_t strlen(char *string) {
    size_t i = 0;
    while (string[i] != '\0')
        i++;
    return i;
}

// 0 sama, 1 beda
uint8_t strcmp(char *s1, char *s2) {
    size_t i = 0;
    if (strlen(s1) == strlen(s2)) {
        while (s1[i] != '\0') {
            if (s1[i] != s2[i])
                return 1;
            i++;
        }
        return 0;
    }
    return 1;
}

void strcpy(char *dst, char *src, int type) {
    size_t i = 0;
    clear(dst, strlen(dst));
    if (type == 1) {
        while (src[i] != '\0') {
            dst[i] = src[i];
            i++;
        }
    } else {
        while (src[i] != 0xA) {
            dst[i] = src[i];
            i++;
        }
    }
}