#include "std/stdtype.h"
#include "std/stdmem.h"
#include "filesystem/fat32.h"

#ifndef _USERSHELL_
#define _USERSHELL_

// Color declarations
#define BIOS_LIGHT_GREEN 0b1010
#define BIOS_GREY        0b0111
#define BIOS_LIGHT_BLUE  0b1001
#define BIOS_RED         0b1100

// Position of current directory
/* extern uint32_t current_directory = ROOT_CLUSTER_NUMBER; */

// Some string and pointer processing libs
// 1. String length
size_t strlen(char *string);
// 2. Clear the pointer buffer
void clear(void* pointer, size_t n);
// 3. Copying the string value
void strcpy(char *dst, char *src, int type);
// 4. Get the parsed args nums and components
int inputparse (char *input, char output[4][64], bool *valid);

// Interrupt to main
void interrupt (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Some function using the base interrupt
// 1. Put chars to screen
void put (char* buf, uint8_t color);

#endif