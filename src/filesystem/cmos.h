#ifndef _CMOS_H
#define _CMOS_H

#include "portio/portio.h"

// macro to define current year
#define CURRENT_YEAR        2023

// macro of CMOS IO port
#define CMOS_ADDRESS        0x70
#define CMOS_DATA           0x71

// global variables needed to save the date and time readed from register
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

// return true if update in register is happening
int get_update_in_progress_flag();

// get the date time value from CMOS register 
unsigned char get_RTC_register(int reg);

// read the current time and update the global variables (second, minute, etc)
void read_rtc();

#endif