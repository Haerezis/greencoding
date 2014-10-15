#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lightup.h"

int main(int argc, char **argv) {
   if (argc < 4) {
      printf("Generates a lightup puzzle and one possible solution\n");
      printf("Usage: %s width height file [seed]\n", argv[0]);
      return EXIT_FAILURE;
   }

   unsigned int width = atoi(argv[1]);
   unsigned int height = atoi(argv[2]);

   if (width < 2 || height < 2) {
      printf("Invalid width or height specified\n");
      return EXIT_FAILURE;
   }

   lu_puzzle *p = puzzle_new(width, height);

   unsigned int seed;
   if (argc > 4) {
      seed = atoi(argv[4]);
   } else {
      seed = (unsigned int) time(NULL);
   }
   srand(seed);

   printf("Seed: %u\n", seed);

   assert(p != NULL);

   // initial step: put one bulb per column to make sure every square is
   // enlightened
   unsigned int i, x, y, prev_y=-1;
   for (x = 0; x < width; ++x) {
      unsigned int min_y, max_y;

      // use any row different from the previous one
      // select the largest continuous row range before or after the
      // previous row used
      // avoids having two adjacent light bulbs
      if (x == 0) {
         min_y = 0;
         max_y = height - 1;
      } else if (prev_y >= height / 2) {
         min_y = 0;
         max_y = prev_y - 1;
      } else {
         min_y = prev_y + 1;
         max_y = height - 1;
      }

      y = min_y + (unsigned int) 
         (((double) rand() / RAND_MAX) * (max_y - min_y + 1));
      prev_y = y;

      p->data[y * width + x] = lusq_lbulb;
   }

   // separate bulbs on the same row by a wall
   for (y = 0; y < height; ++y) {
      unsigned int prev_x = width;

      for (x = 0; x < width; ++x) {
         if (p->data[y * width + x] == lusq_lbulb) {
            // first bulb on row
            if (prev_x == width) {
               prev_x = x;

            // next bulbs
            } else {
               unsigned int wall_x;

               // separate bulbs by a wall anywhere in between
               wall_x = prev_x + 1 + (unsigned int)
                  (((double) rand() / RAND_MAX) * (x - prev_x - 1));
               p->data[y * width + wall_x] = lusq_block_any;

               prev_x = x;
            }
         }
      }
   }

   // also randomly add some walls
   for (i = 0; i < width * height; ++i) {
      if (p->data[i] != lusq_empty) {
         continue;
      }

      if (((double) rand() / RAND_MAX) <= 0.05) {
         p->data[i] = lusq_block_any;
      }
   }

   puzzle_lights_on(p);

   // reach a certain ratio of empty cells
   // remember: empty cells at that point will be converted into walls later
   // empty cells in the final solutions are currently in the enlightened state
   unsigned int nb_empty = puzzle_count(p, lusq_empty);
   while ((double) nb_empty / (height * width) > 0.15) {
      unsigned int cell_pos = ((double) rand() / RAND_MAX) * (nb_empty - 1);

      // insert a light somewhere
      for (y = 0; y < height; ++y) {
         for (x = 0; x < width; ++x) {
            if (p->data[y * width + x] != lusq_empty) {
               continue;
            }

            if (cell_pos == 0) {
               p->data[y * width + x] = lusq_lbulb;

               puzzle_light_on(p, x, y);

               y = height;
               break;
            } else {
               --cell_pos;
            }
         }
      }

      //printf("%u/%u: %.2f\n", nb_empty, width * height,
      //    (double) nb_empty / (height * width));
      nb_empty = puzzle_count(p, lusq_empty);
   }

   // convert the remaining empty squares and walls into constrained walls
   for (y = 0; y < height; ++y) {
      for (x = 0; x < width; ++x) {
         i = y * width + x;

         if (p->data[i] != lusq_empty && p->data[i] != lusq_block_any) {
            continue;
         }

         unsigned int lbcount = 0, wcount = 0;
         if (x >= 1) {
            lbcount += p->data[i - 1] == lusq_lbulb;
            wcount += p->data[i - 1] == lusq_empty
               || p->data[i - 1] == lusq_block_any;
         }
         if (x < width - 1) {
            lbcount += p->data[i + 1] == lusq_lbulb;
            wcount += p->data[i + 1] == lusq_empty
               || p->data[i + 1] == lusq_block_any;
         }
         if (y >= 1) {
            lbcount += p->data[i - width] == lusq_lbulb;
            wcount += p->data[i - width] == lusq_empty
               || p->data[i - width] == lusq_block_any;
         }
         if (y < height - 1) {
            lbcount += p->data[i + width] == lusq_lbulb;
            wcount += p->data[i + width] == lusq_empty
               || p->data[i + width] == lusq_block_any;
         }

         // hints are useless if the wall is surrounded by other walls
         if (wcount == 4) {
            p->data[i] = lusq_block_any;
            continue;
         }

         // the higher the number of neighboring bulbs, the more chances we 
         // have to constrain the wall
         if ((1 + lbcount) * ((double) rand() / RAND_MAX) < 0.9) {
            p->data[i] = lusq_block_any;
         } else {
            p->data[i] = (lu_square) lbcount;
         }
      }
   }

   puzzle_lights_off(p);

   // save the original solution
   char *solpath = (char *) malloc((strlen(argv[3]) + 5) * sizeof(char));
   strcpy(solpath, argv[3]);
   strcat(solpath, ".sol");
   FILE *fds = puzzle_open_storage_file(solpath, width, height);
   puzzle_store(p, fds);
   fclose(fds);
   free(solpath);

   // remove the light bulbs
   for (i = 0; i < width * height; ++i) {
      if (p->data[i] == lusq_lbulb) {
         p->data[i] = lusq_empty;
      }
   }

   puzzle_print(p);
   
   // save the problem
   char *pbpath = (char *) malloc((strlen(argv[3]) + 4) * sizeof(char));
   strcpy(pbpath, argv[3]);
   strcat(pbpath, ".pb");
   FILE *fdp = puzzle_open_storage_file(pbpath, width, height);
   puzzle_store(p, fdp);
   fclose(fdp);
   free(pbpath);

   puzzle_destroy(p);
   
   return EXIT_SUCCESS;
}
