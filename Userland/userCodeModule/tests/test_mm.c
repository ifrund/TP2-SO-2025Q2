#include "test_util.h"

#define MAX_BLOCKS 128

typedef struct MM_rq {
  void *address;
  uint32_t size;
} mm_rq;


void * memset2(void * destiation, int32_t c, uint64_t length) {
  //write_out("entro a memset2--");
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;
  //write_out("antes del copiado--");

	while(length--)
		dst[length] = chr;
  //write_out("dsp del copiado (memset2)\n");

	return destiation;
}

//Toma como parámetro la cantidad máxima de memoria a utilizar en bytes.
void test_mm_dummy(int argc, char **argv) {

  create_mm();

  mm_rq mm_rqs[MAX_BLOCKS];
  uint8_t rq;
  uint32_t total;
  uint64_t max_memory;

  if (argc != 1){
    write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 argumento.\n");
    exit_pcs(ERROR);
  }

  if ((max_memory = satoi(argv[0])) <= 0){
    write_out("error en el satoi\n");
    exit_pcs(ERROR);
  }
  int c = 0;

  while (1) {
    rq = 0;
    total = 0;
    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < max_memory) {
      mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
      //write_out("Voy a hacers alloc--");
      mm_rqs[rq].address = _alloc(mm_rqs[rq].size);
      c++;      

      if (mm_rqs[rq].address != NULL) {
        total += mm_rqs[rq].size;
        rq++;
        //printDec(c);
        //write_out("-");
      }
      else{
        write_out("error en el alloc, vuelta: ");
        printDec(c);
        write_out("\n");
        for (int i = 0; i < rq; i++){
          if (mm_rqs[i].address != NULL){
            _free(mm_rqs[i].address);
            //write_out("Libero la memoria ");
          }
        }
        exit_pcs(ERROR);
      }
    }
    //write_out("Terminamos de hacer Alloc\n");

    // Set
    uint32_t i;
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL){
        //write_out("Vamos a hacer memset2--");
        memset2(mm_rqs[i].address, i, mm_rqs[i].size);
        //write_out("Hicimos memset2\n");
      } 

    //write_out("Terminamos de hacer memset\n"); 
    // Check
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL)
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
          write_out("test_mm ERROR\n");
          exit_pcs(ERROR);
        }
    //write_out("Terminamos de hacer memCheck\n"); 
                    
    // Free
    for (i = 0; i < rq; i++){
      if (mm_rqs[i].address != NULL){
        _free(mm_rqs[i].address);
        //write_out("Libero la memoria ");
      }
    }
    //write_out("\n");

  }
}
