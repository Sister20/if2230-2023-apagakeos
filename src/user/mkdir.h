// File : mkdir.h
// Header for mkdir.h, contains the declaration of functions needed to process mkdir command

#ifndef _MKDIR_H_
#define _MKDIR_H_

void createDirectory(char* args_val, int (*args_info)[2], int args_pos);

void mkdir(char* args_val, int (*args_info)[2], int args_count);

#endif