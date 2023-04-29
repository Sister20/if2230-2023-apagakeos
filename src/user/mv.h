// File : mv.h
// Header for mv.c, contains the declaration of functions needed to process mv command

#ifndef _MV_H_
#define _MV_H_

bool remove_mv(char* args_val, int (*args_info)[2], int args_count);

bool copy_mv(char* args_val, int (*args_info)[2], int args_count);

int parse(char* args_val, int (*args_info)[2], int args_count, int cluster, char* _name, char* _ext);

void rename(char* args_val, int (*args_info)[2], int args_count, int cluster_1, int cluster_2);

void move(char* args_val, int (*args_info)[2], int args_count, int cluster_1, int cluster_2);

void mv(char* args_val, int (*args_info)[2], int args_count);

#endif