// File : mkdir.h
// Header for mkdir.c, contains the declaration of functions needed to process mkdir command

#ifndef _MKDIR_H_
#define _MKDIR_H_

#include "std/stdtype.h"
#include "std/stdmem.h"

void mkdir(char* args_val, char (*args_info)[2], int args_count);

#endif