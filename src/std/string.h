// Implmenetasi library pemrosesan string
#include "stdtype.h"

size_t strlen(const char *str);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);

char *strcpy_safe(char *dst, const char *src, size_t dstSize);