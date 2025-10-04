#include <stdint.h>
#include <time.h>
#include <lib.h>

// Esto fue sacado de la pagina de wikiOSDev: https://wiki.osdev.org/PC_Speaker

 //Play sound using built in speaker
 static void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
    //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	_outb(0x43, 0xb6);                        // b0 BCD, b3b2b1 "SQUARE WAVE GEN", b5b4 access lobyte/hibyte, b7b6 Channel2
 	_outb(0x42, (uint8_t) (Div) );            // 42h is Channel2 port, first low 
 	_outb(0x42, (uint8_t) (Div >> 8));        // then high
 
    //And play the sound using the PC speaker
 	tmp = _inb(0x61);                         // recupero el valor del PIT port
  	if (tmp != (tmp | 3)) {                   // aseguramos bit 0 = 1 (connected with PIT high), y bit 1 = 1 (move position to out)
 		_outb(0x61, tmp | 3);
 	}
 }
 
 //make it shutup
 static void shutup() {
 	uint8_t tmp = _inb(0x61) & 0xFC; // tomo lo que estaba en el reg y le hago and para que los ultimos dos bits sean 0
  	_outb(0x61, tmp); 
 }
 
 //Make a beep
 void beep(uint32_t frequency, int duration) {
 	 play_sound(frequency);
 	 sleep(duration, 1);
 	 shutup();
 }

void beep_asm(){
    beep(1000, 1);
}
