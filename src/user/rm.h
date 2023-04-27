// File : rm.h
// Header for rm.c, contains the declaration of functions needed to process rm command

#ifndef _RM_H_
#define _RM_H_

void remove(char* args_val, int (*args_info)[2], int args_count);

void rm(char* args_val, int (*args_info)[2], int args_count);

#endif