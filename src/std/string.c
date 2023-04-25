#include "string.h"

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