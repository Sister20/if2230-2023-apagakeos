// File : whereis.h
// Header for whereis.c, contains the declaration of functions needed to process whereis command

#ifndef _WHEREIS_H_
#define _WHEREIS_H_

#include "std/stdtype.h"

void processDFS (char srcName[8], uint32_t search_directory_number, int v, bool visited[63]);

void doWhereis (char* args_val, int (*args_info)[2], int args_pos);

void whereis (char* args_val, int (*args_info)[2], int args_count);

#endif