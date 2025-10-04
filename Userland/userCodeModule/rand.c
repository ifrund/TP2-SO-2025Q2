#include "include/rand.h"
#include "include/userlib.h"

//using https://wiki.osdev.org/Random_Number_Generator that cites
//https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf
//for pseudorandom number generators

unsigned long int next=1;

int rand(){
    next = next * 1103515245 + 12345;
    return (unsigned int) (next/65536) % 32768;//this because of RAND_MAX value
}

void srand(unsigned int seed){
    next=seed;
}

unsigned int time(){
    int hrs=0, min=0, seg=0;
    getClock(&hrs,&min,&seg);
    return (hrs*(18*60*60)) + (min*(18*60)) + (seg*(18));
}