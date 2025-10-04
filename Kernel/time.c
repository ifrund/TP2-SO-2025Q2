#include <lib.h>
#include <naiveConsole.h>
#include <stdint.h>
#include <time.h>
#include <interrupts.h>

#define MILLENIUM 2000 // cambiarlo si llega a ser necesario
#define CENTURY 0      // lo usamos para calcular la fecha completa

static unsigned long ticks = 0;
uint8_t *clockLocation = (uint8_t *)0xB808E;

char *dayNames[] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};

unsigned long ticks_elapsed() { return ticks; }

long long nanos_elapsed(){
  return ((long long)ticks*1000000000/18);
}

long long milis_elapsed(){
  return ((long long)ticks*1000/18);
}

unsigned long seconds_elapsed() { return ticks / 18; }

void update_clock() {
  if (ticks % 18 == 0) {

    uint8_t *current = getCurrentVideo();
    setCurrentVideo(clockLocation);
    //printTime();
    setCurrentVideo(current);
  }
}

void set_clock_location(uint8_t *location) { clockLocation = location; }

void formatTime(uint8_t *hour, uint8_t *min, uint8_t *sec) {
  *sec = _getSeconds();
  *min = _getMinutes();
  *hour = _getHours();

  // nos fijamos si hay que corregir el formato
  uint8_t format = _getDateTimeFormat();
  if ((!(format & 0x02)) && (*hour & 0x80)) { // si esta en 12hs, lo pasamos a
                                              // 24
    *hour = ((*hour & 0x7F) + 12) % 24;
  }
  if (!(format & 0x04)) { // si esta en BCD pasamos a decimal
    *sec = (*sec & 0xF) + ((*sec / 16) * 10);
    *min = (*min & 0xF) + ((*min / 16) * 10);
    *hour = (*hour & 0xF) + ((*hour / 16) * 10);
  }

  // la hora viene en UTC, por lo que la asignamos en UTC-3 para Argentina
  // (ignoramos la lógica del caso con daylight savings)

  if (*hour >= 3)
    *hour -= 3;
  else {
    *hour = *hour + 21;
  }
}

void formatDate(uint8_t *dayWeek, uint8_t *dayMonth, uint8_t *month,
                uint16_t *year) {
  *dayWeek = _getDayWeek();
  *dayMonth = _getDayMonth();
  *month = _getMonth();
  *year = _getYear(); // solo trae los dos ultimos numeros
  uint8_t hour = _getHours();

  // nos fijamos si hay que corregir el formato
  uint8_t format = _getDateTimeFormat();
  if (!(format & 0x04)) { // si esta en BCD pasamos a decimal (dayWeek no hace
                          // falta porque siempre es entre 1-7)
    *dayMonth = (*dayMonth & 0xF) + ((*dayMonth / 16) * 10);
    *month = (*month & 0xF) + ((*month / 16) * 10);
    *year = (*year & 0xF) + ((*year / 16) * 10);
  }

  // colocamos completo el año
  *year += MILLENIUM;
  *year += CENTURY;

  // esta en UTC, por lo que tengo que corregir la fecha segun hora, etc.
  if (hour < 3) {
    if (*dayMonth == 1) { // caso cambio de mes
      if (*month == 1) {  // caso cambio de año
        *year = *year - 1;
        *month = 12;
      } else
        *month = *month - 1;
      *dayMonth = calculateMonthLastDay(*month, *year);
    } else
      *dayMonth = *dayMonth - 1;
    *dayWeek = ((*dayWeek + 5) % 7) + 1; // siempre va a ser el anterior
  }
}

// calcula el ultimo dia del mes dado
uint8_t calculateMonthLastDay(uint8_t month, uint16_t year) {
  if (month == 2) { // hay que fijarse si es bisiesto
    if ((year % 4 == 0) || (year % 100 != 0) || (year % 400 == 0))
      return 29;
    else
      return 28;
  } else {
    if ((month % 2 == 0 && month < 8) || (month % 2 == 1 && month > 8))
      return 30;
    else
      return 31;
  }
}

void printTime(int *hrs, int *min, int *seg){
     
    uint8_t segs, hrss, mins;
    formatTime(&hrss, &mins, &segs);
    *hrs=hrss;
    *min=mins;
    *seg=segs;
    
    //no tiene sentido, en esta linea por alguna razón solo andan bien los min
    //formatTime((uint32_t*)hrs, (uint32_t*)min, (uint32_t*)seg);
}

void printDate(){
	uint8_t dayWeek, dayMonth, month;
	uint16_t year;
	formatDate(&dayWeek, &dayMonth, &month, &year);

	ncPrint("Date: ");
	ncPrint(dayNames[dayWeek-1]);
	ncPrint(" ");
	if(dayMonth<10) ncPrintDec(0);
	ncPrintDec(dayMonth);
	ncPrint("/");
	if(month<10) ncPrintDec(0);
	ncPrintDec(month);
	ncPrint("/");
	ncPrintDec(year);
	ncNewline();
}

//================================================================================================================================
// Sleep
//================================================================================================================================

void timer_handler() {
  ticks++;
  //update_clock();
}

void sleep(int sec, int uni){
  if (sec == 0){
    _hlt();
    return;
  }
  
  unsigned long t0 = ticks_elapsed();
  while(((ticks_elapsed()-t0)*pow(1000, uni))/18 < sec ) _hlt(); 
}
