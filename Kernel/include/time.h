#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

void timer_handler();

unsigned long ticks_elapsed();

void update_clock();
void set_clock_location(uint8_t *location);
void formatTime(uint8_t *hour, uint8_t *min, uint8_t *sec);
void formatDate(uint8_t *dayWeek, uint8_t *dayMonth, uint8_t *month, uint16_t *year);
uint8_t calculateMonthLastDay(uint8_t month, uint16_t year);
void printTime(int *hrs, int *min, int *seg);
void printDate();

void sleep(int sec, int uni);
void my_ints();

#endif
