#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lightup.h"
#include "lightupsolver.h"
#include "utils.h"
#include "heuristics.h"
#include "solve.h"

extern position_array left_, right_ ,top_ ,bottom_,center_;

int solver_main(int argc, char **argv) {
   if (argc < 3) {
      printf("Solves a ligthup puzzle\n");
      printf("Usage: %s <problem> <output_file>\n", argv[0]);
      return EXIT_FAILURE;
   }

   disable_cpu(2);//XXX

   printf("Loading %s for solving...\n", argv[1]);
   lu_puzzle *p = puzzle_load(argv[1], 1);

   FILE *fd = puzzle_open_storage_file(argv[2], p->width, p->height);
   printf("Solving...\n");

   unsigned int nb_e = puzzle_count(p, lusq_empty);
   printf("Problem size = %u\n", nb_e);

   unsigned int sol_id = 0;
   position_array positions_empty,positions_impossible, left, right, top, bottom, center;
   position_array *pa_array_empty;
   position_array *pa_array_impossible;
   unsigned int pa_array_size;
   
   pre_solve(p, &positions_empty, &positions_impossible, &left, &right, &top, &bottom, &center);
   //puzzle_print(p);
   //FIXME
   left_ = left;
   right_ = right;
   top_ = top;
   bottom_ = bottom;
   center_ = center;

   classify_positions(p, &pa_array_empty, &pa_array_impossible, &pa_array_size, positions_empty);
   //print_class(pa_array_empty,pa_array_size);
   //printf("\n");
   //print_class(pa_array_impossible,pa_array_size);
   
   printf("Problem size after heuristic = %u\n", positions_impossible.size + positions_empty.size);

   solve_classes(p, pa_array_empty, pa_array_impossible, pa_array_size, &sol_id, fd);

   printf("Found %u solutions\n", sol_id);

   puzzle_destroy(p);
   fclose(fd);

   return EXIT_SUCCESS;
}

#if !defined (__BUILD_FOR_USE_WITH_ISDA__) || defined (__BUILD_FOR_USE_WITH_ISDA_AS_STANDALONE_TOOL__)

int main(int argc, char **argv) {
   return solver_main(argc, argv);
}

#endif

