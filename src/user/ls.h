// File : ls.h
// Header for ls.c, contains the declaration of functions needed to process ls command

#ifndef _LS_H_
#define _LS_H_

void printDirectoryTable();

void access(char* args_val, int (*args_info)[2], int args_pos);

void ls(char* args_val, int (*args_info)[2], int args_count);

#endif