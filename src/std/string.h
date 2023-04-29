// Implementasi library pemrosesan string
#include "stdtype.h"

#ifndef _STRINGH_
#define _STRINGH_

void clear(void *pointer, size_t n);

size_t strlen(char *string);

uint8_t strcmp(char *s1, char *s2);

void strcpy(char *dst, char *src, int type);

#endif