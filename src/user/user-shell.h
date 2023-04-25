#ifndef _USERSHELL_
#define _USERSHELL_

#include "std/stdtype.h"
#include "std/stdmem.h"
#include "filesystem/fat32.h"
#include "std/string.h"

// Color declarations
#define BIOS_LIGHT_GREEN 0b1010
#define BIOS_GREY        0b0111
#define BIOS_LIGHT_BLUE  0b1001
#define BIOS_RED         0b1100

// Position of current directory
extern uint32_t current_directory;

// Get the parsed args nums and components
int inputparse (char *args_val, char args_info[4][2]);

// Interrupt to main
void interrupt (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Some function using the base interrupt
// Put chars to screen
void put (char* buf, uint8_t color);
void putn(char* buf, uint8_t color, int n);

// Print Current Working Directory
void printCWD (char* path_str, uint32_t current_dir);

void processCommand(char* args_val, char (*args_info)[2], int args_count);

#endif