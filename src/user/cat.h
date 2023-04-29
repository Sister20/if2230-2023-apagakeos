// File : cat.h
// Header for cat.c, contains the declaration of functions needed to process cat command

#ifndef _CAT_H_
#define _CAT_H_

void showFiles (char* args_val, int (*args_info)[2], int args_pos);

void cat (char* args_val, int (*args_info)[2], int args_count);

#endif