#include "test_util.h"
#include "../include/userlib.h" //create_mm lib, not sure why this was commented
#include "../include/shell.h"   //write_out lib, for testing
//#include <string.h>

#define MAX_BLOCKS 128

typedef struct MM_rq {
  void *address;
  uint32_t size;
} mm_rq;


void * memset2(void * destiation, int32_t c, uint64_t length) {
  write_out("entro a memset2\n");
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;
  write_out("antes del copiado\n");

	while(length--)
		dst[length] = chr;

  write_out("dsp del copiado (memset2)\n");

	return destiation;
}

uint64_t test_mm(uint64_t argc, char *argv[]) {

  create_mm();

  mm_rq mm_rqs[MAX_BLOCKS];
  uint8_t rq;
  uint32_t total;
  uint64_t max_memory;

  if (argc != 1)
    return -1;

  if ((max_memory = satoi(argv[0])) <= 0)
    return -1;

  while (1) {
    rq = 0;
    total = 0;

    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < max_memory) {
      mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
      write_out("Voy a hacers alloc\n");
      mm_rqs[rq].address = alloc(mm_rqs[rq].size);
      

      if (mm_rqs[rq].address) {
        total += mm_rqs[rq].size;
        rq++;
        write_out("Funciono alloc\n");
      }
    }
    write_out("Terminamos de hacer Alloc\n");

    // Set
    uint32_t i;
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address){
        write_out("Vamos a hacer memset2\n");
        memset2(mm_rqs[i].address, i, mm_rqs[i].size);
        write_out("Hicimos memset2\n");
      } 

    write_out("Terminamos de hacer memset\n"); 
    // Check
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address)
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
          write_out("test_mm ERROR\n");
          return -1;
        }
    write_out("Terminamos de hacer memCheck\n"); 
    // Free
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address){
        free(mm_rqs[i].address);
        write_out("Libero la memoria");
      }
  }
}
