#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lightup.h"
#include "lightupchecker.h"

#define HASHTABLE_SIZE 1024

// hashtable de puzzle
static puzzle_list *puzzle_ht[HASHTABLE_SIZE] = { NULL };

static void free_table() {
   unsigned int i;

   for (i = 0; i < HASHTABLE_SIZE; ++i) {
      puzzle_list *pl;
      for (pl = puzzle_ht[i]; pl != NULL;) {
         puzzle_destroy(pl->puzzle);

         puzzle_list *lpn = pl->next;
         free(pl);
         pl = lpn;
      }
   }
}

int checker_main(int argc, char **argv) {
   if (argc < 4) {
      printf("Checks a given lightup puzzle\n");
      printf("Usage: %s reference sol_to_check ref_solution\n", argv[0]);
      printf("Returns 1 if the puzzle solution is valid, 0 otherwise\n");
      return 0;
   }

   lu_puzzle *ref = puzzle_load(argv[1],1);
   
   // vérifie les solutions
   unsigned int i = 0, width, height;
   FILE *fd;
   lu_puzzle *sol;

   fd = puzzle_open_sol_file(argv[2], &width, &height);
   if(fd==NULL)
      return 0;
   while ((sol = puzzle_load_next_sol(fd, width, height))) {
      ++i;

      // la solution est correcte ?
      if (!puzzle_check(ref, sol)) {
         printf("Solution %d is invalid\n", i);
         free_table();
         puzzle_destroy(sol);
         puzzle_destroy(ref);
         fclose(fd);
         return 0;
      }

      unsigned int hash = puzzle_hash(sol) % HASHTABLE_SIZE;
      puzzle_list *npl = (puzzle_list *) malloc(sizeof(*npl));
      npl->next = NULL;
      npl->puzzle = sol;

      if (puzzle_ht[hash] == NULL) {
         puzzle_ht[hash] = npl;
      } else {
         puzzle_list *pl;

         // vérifie si on a déjà vu la solution
         for (pl = puzzle_ht[hash]; pl != NULL; pl = pl->next) {
            if (puzzle_eq(sol, pl->puzzle)) {
               printf("Solution %d met at least twice\n", i);
               free(npl);
               free_table();
               puzzle_destroy(sol);
               puzzle_destroy(ref);
               fclose(fd);
               return 0;
            }
         }

         // on stocke la solution dans la hashtable
         npl->next = puzzle_ht[hash];
         puzzle_ht[hash] = npl;
      }
   }

   free_table();
   puzzle_destroy(ref);
   fclose(fd);

   FILE *fd1 = puzzle_open_sol_file(argv[3], &width, &height);
   if(fd1==NULL)
      return 0;
   fseek(fd1,0,SEEK_END);
   unsigned int nb_known_sols = (ftell(fd1)-2*sizeof(unsigned int))/(width*height*sizeof(lu_square));
   fclose(fd1);

   if (i < nb_known_sols) {
      printf("Invalid number of solutions: expecting %d but %d given\n",
             nb_known_sols, i);
      return 0;
   } else if (i > nb_known_sols) {
      printf("You found more (%d expected) valid solutions than the reference implementation on the problem \"%s\"\n", nb_known_sols, argv [1]);
      return 0;
   }

   printf("The puzzle solutions are correct!\n");

   return 1;
}

#ifndef __BUILD_FOR_USE_WITH_ISDA__

int main(int argc, char **argv) {
   return checker_main(argc, argv);
}

#endif
