// File : mv.h
// Header for mv.c, contains the declaration of functions needed to process mv command

#ifndef _MV_H_
#define _MV_H_

#include "std/stdtype.h"

bool remove_mv(char* args_val, int (*args_info)[2], int args_count);

bool copy_mv(char* args_val, int (*args_info)[2], int args_count);

void mv(char* args_val, int (*args_info)[2], int args_count);

#endif