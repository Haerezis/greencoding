#include "lightup.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
   int x;
   if (argc < 2 || argc>4 ) {
      printf("Prints a lightup puzzle\n");
      printf("Usage: %s filename\n", argv[0]);
      return EXIT_FAILURE;
   }
   if( argc==3 )
   {
      x = atoi(argv[2]);
   }
   else
   {
      x = 1;
   }

   lu_puzzle *p = puzzle_load(argv[1],x);
   puzzle_print(p);
   puzzle_destroy(p);

   return EXIT_SUCCESS;
}
