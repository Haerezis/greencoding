#include "lightup.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// !!! pas de correction d'endianness pour les fonctions I/O

FILE *puzzle_open_sol_file(const char *path, unsigned int *width, unsigned int *height)
{
	FILE *fd = fopen(path, "r");
	if (fd == NULL)
	{
		perror("Cannot open puzzle file");
		return NULL;
	}
	if( fread( width, sizeof(*width), 1, fd ) != 1 )
	{
		fprintf(stderr, "Invalid file format for file %s\n", path);
		fclose(fd);
		return NULL;
	}
	if( fread( height, sizeof(*height), 1, fd ) != 1 )
	{
		fprintf(stderr, "Invalid file format for file %s\n", path);
		fclose(fd);
		return NULL;
	}
	return( fd );
}
lu_puzzle *puzzle_load_next_sol(FILE *fd, const unsigned int width, const unsigned int height) 
{

   // lecture des données
   lu_square *data = (lu_square *) malloc(width * height * sizeof(*data));
   if (fread(data, sizeof(*data), width * height, fd) != width * height)
   {
      free(data);
      return NULL;
   }

   lu_puzzle *res = (lu_puzzle *) malloc(sizeof(*res));
   res->width = width;
   res->height = height;
   res->data = data;

   return res;
}
lu_puzzle *puzzle_load(const char *path, const unsigned int no) 
{
   FILE *fd = fopen(path, "r");

   if (fd == NULL) {
      perror("Cannot open puzzle file");
      return NULL;
   }

   // lecture des coordonnées
   lu_puzzle *res = (lu_puzzle *) malloc(sizeof(*res));
   if (fread(&res->width, sizeof(res->width), 1, fd) != 1) {
      fprintf(stderr, "Invalid file format for file %s\n", path);
      free(res);
      fclose(fd);
      return NULL;
   }
   if (fread(&res->height, sizeof(res->height), 1, fd) != 1) {
      fprintf(stderr, "Invalid file format for file %s\n", path);
      free(res);
      fclose(fd);
      return NULL;
   }

   if( no!=1 )
   {
     if( fseek(fd, (no-1)*res->width*res->height*sizeof(unsigned char), SEEK_CUR ) < 0 )
     {
        perror( "fseek" );
        fprintf( stderr, "invalid puzzle number %d\n", no );
        free( res );
        fclose( fd );
        return( NULL );
     }
   }
   // lecture des données
   res->data = (lu_square *) malloc(res->width * res->height * sizeof(*res->data));
   if (fread(res->data, sizeof(*res->data), res->width * res->height, fd)
       != res->width * res->height)
   {
      fprintf(stderr, "Invalid file size for %s\n", path);
      free(res->data);
      free(res);
      fclose(fd);
      return NULL;
   }

   fclose(fd);

   return res;
}

FILE *puzzle_open_storage_file(char *fname, unsigned int width, unsigned int height) {
   // on crée le fichier de résultat
   FILE *fd;
   if (!(fd=fopen(fname,"w"))) {
      perror("opening result file");
      return NULL;
   }
   // et on y écrit la largeur/hauteur des puzzles
   if (fwrite(&width, sizeof(width), 1, fd) != 1) {
      fprintf(stderr, "Cannot write puzzle width\n");
      fclose(fd);
      return NULL;
   }
   if (fwrite(&height, sizeof(height), 1, fd) != 1) {
      fprintf(stderr, "Cannot write puzzle height\n");
      fclose(fd);
      return NULL;
   }
   return fd;
}

void puzzle_store(const lu_puzzle *p, FILE *fd) {
   if (p == NULL) {
      fprintf(stderr, "Store puzzle called on an empty puzzle\n");
      return;
   }
   if (fd == NULL) {
      perror("Cannot open puzzle file for writing");
      return;
   }

   // écriture des données
   if (fwrite(p->data, sizeof(*p->data), p->width * p->height, fd)
       != p->width * p->height)
   {
      perror("Cannot write puzzle data");
      fclose(fd);
      return;
   }
}

void puzzle_destroy(lu_puzzle *p) {
   if (p != NULL) {
      free(p->data);
      free(p);
   }
}

static char prettychar [] =
{
        '0',
        '1',
        '2',
        '3',
        '4',
        '#',
        ' ',
        '?',
        '.',
        'X',
        '@',
        '_',
};

void puzzle_print(const lu_puzzle *p) {
        unsigned int x, y;

        if (p == NULL) {
           fprintf(stderr, "Cannot print empty puzzles\n");
           return;
        }

        printf("   ");
        for (x = 0; x < p->width; ++x)
                printf("%d", x%10);
        putchar( '\n' );

        for (y = 0; y < p->height; ++y)
        {
                printf( "%02d ", y );
                for (x = 0; x < p->width; ++x)
                        putchar( prettychar[p->data[y * p->width + x]] );
                putchar( '\n' );
        }
        printf("   ");
        for (x = 0; x < p->width; ++x)
                printf("%d", x%10);
        putchar( '\n' );

}

lu_puzzle *puzzle_new(unsigned int width, unsigned int height) {
   lu_puzzle *res = (lu_puzzle *) malloc(sizeof(*res));

   if (width == 0 || height == 0) {
      fprintf(stderr, "Invalid dimensions for the new puzzle: %u %u\n", width,
              height);
      return NULL;
   }

   res->width = width;
   res->height = height;
   res->data = (lu_square *) malloc(sizeof(*res->data) * width * height);

   unsigned int i;
   for (i = 0; i < width * height; ++i) {
      res->data[i] = lusq_empty;
   }

   return res;
}

lu_puzzle *puzzle_clone(const lu_puzzle *p) {
   if (p == NULL) {
      return NULL;
   }

   lu_puzzle *res = (lu_puzzle *) malloc(sizeof(*res));
   res->width = p->width;
   res->height = p->height;
   res->data = (lu_square *) malloc((res->width * res->height) * sizeof(*res->data));

   memcpy(res->data, p->data, res->width * res->height * sizeof(*res->data));

   return res;
}

void puzzle_lights_on(lu_puzzle *p) {
   int x, y, width, height;

   if (p == NULL) {
      fprintf(stderr, "Cannot enlight an empty puzzle\n");
      return;
   }

   width = p->width;
   height = p->height;
   
   for (y = 0; y < height; ++y) {
      for (x = 0; x < width; ++x) {
         if (p->data[y * width + x] != lusq_lbulb) {
            continue;
         }

         puzzle_light_on(p, x, y);
      }
   }
}

__inline void puzzle_light_on(lu_puzzle *p, unsigned int x, unsigned int y)
{
   if (p == NULL) {
      fprintf(stderr, "Cannot find a light in an empty puzzle\n");
      return;
   }

   if (x >= p->width || y >= p->height) {
      fprintf(stderr, "Invalid coordinates %u %u\n", x, y);
      return;
   }
/*
   if (p->data[y * p->width + x] != lusq_lbulb) {
      fprintf(stderr, "No light to turn on at %u %u\n", x, y);
      return;
   }
*/
   p->data[y * p->width + x] = lusq_lbulb;
   int width = p->width;
   int height = p->height;

   // éclairage dans les 4 directions

   int x2, y2;

   // vers la droite
   for (x2 = x + 1; x2 < width; ++x2) {
      int i = y * width + x2;

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible && p->data[i] != lusq_enlighted) {
         break;   // mur
      }

      p->data[i] = lusq_enlighted;
   }
   // vers la gauche
   for (x2 = (int) x - 1; x2 >= 0; --x2) {
      int i = y * width + x2;

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible && p->data[i] != lusq_enlighted) {
         break;   // mur
      }

      p->data[i] = lusq_enlighted;
   }
   // vers le bas
   for (y2 = y + 1; y2 < height; ++y2) {
      int i = y2 * width + x;

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible && p->data[i] != lusq_enlighted) {
         break;   // mur
      }

      p->data[i] = lusq_enlighted;
   }
   // vers le haut
   for (y2 = (int) y - 1; y2 >= 0; --y2) {
      int i = y2 * width + x;

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible && p->data[i] != lusq_enlighted) {
         break;   // mur
      }

      p->data[i] = lusq_enlighted;
   }
}


__inline void puzzle_light_on_with_bufs(lu_puzzle *p, unsigned int x, unsigned int y, char * wbuf, char * hbuf, int * empty_count)
{
   unsigned int width = p->width;
   unsigned int height = p->height;
   int empty_count_local = 0;
   /*
   if (p == NULL) {
      fprintf(stderr, "Cannot find a light in an empty puzzle\n");
      return;
   }

   if (x >= p->width || y >= p->height) {
      fprintf(stderr, "Invalid coordinates %u %u\n", x, y);
      return;
   }
   */

   p->data[y * p->width + x] = lusq_lbulb;
   empty_count_local++;

   // éclairage dans les 4 directions

   int x2, y2;

   // on éclaire à gauche
   for (x2 = (int) x - 1; x2 >= 0; --x2) {
      int i = y * width + x2;

      if (p->data[i] == lusq_enlighted) {
         continue;
      }

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible) {
         break;   // mur
      }

      wbuf[x2] = p->data[i];
      p->data[i] = lusq_enlighted;      
      empty_count_local++;
   }

   // à droite
   for (x2 = x + 1; (unsigned int) x2 < width; ++x2) {
      int i = y * width + x2;

      if (p->data[i] == lusq_enlighted) {
         continue;
      }

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible) {
         break;   // mur
      }

      wbuf[x2] = p->data[i];
      p->data[i] = lusq_enlighted;
      empty_count_local++;
   }

   // en haut
   for (y2 = (int) y - 1; y2 >= 0; --y2) {
      int i = y2 * width + x;

      if (p->data[i] == lusq_enlighted) {
         continue;
      }

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible) {
         break; // mur
      }

      hbuf[y2] = p->data[i];
      p->data[i] = lusq_enlighted;
      empty_count_local++;
   }

   // en bas
   for (y2 = y + 1; (unsigned int) y2 < height; ++y2) {
      int i = y2 * width + x;

      if (p->data[i] == lusq_enlighted) {
         continue;
      }

      if (p->data[i] != lusq_empty && p->data[i] != lusq_impossible) {
         break; // mur
      }

      hbuf[y2] = p->data[i];
      p->data[i] = lusq_enlighted;
      empty_count_local++;
   }
   *empty_count = *empty_count - empty_count_local;
}


__inline void puzzle_light_off_with_bufs(lu_puzzle *p, unsigned int x, unsigned int y,char * wbuf, char * hbuf, int * empty_count)
{
   int width = p->width;
   int height = p->height;
   int empty_count_local = 0;

   int x2, y2;
/*
   if (p == NULL) {
      fprintf(stderr, "Cannot find a light in an empty puzzle\n");
      return;
   }

   if (x >= p->width || y >= p->height) {
      fprintf(stderr, "Invalid coordinates %u %u\n", x, y);
      return;
   }

   if(p->data[y * p->width + x] != lusq_lbulb)
   {
       fprintf(stderr, "A lightbulb is not on at this case\n");
       return;
   }
   */

   p->data[y * p->width + x] = lusq_empty;
   empty_count_local++;
   
   // vers la droite
   for (x2 = x + 1; x2 < width; ++x2) {
      int i = y * width + x2;

      if (p->data[i] <= lusq_block_any) {
         break;   // mur
      }
      if(wbuf[x2] > 0)
      {
          p->data[i] = wbuf[x2];
          empty_count_local++;
      }
   }
   // vers la gauche
   for (x2 = (int) x - 1; x2 >= 0; --x2) {
      int i = y * width + x2;

      if (p->data[i] <= lusq_block_any) {
         break;   // mur
      }
      if(wbuf[x2] > 0)
      {
          p->data[i] = wbuf[x2];
          empty_count_local++;
      }
   }
   // vers le bas
   for (y2 = y + 1; y2 < height; ++y2) {
      int i = y2 * width + x;

      if (p->data[i] <= lusq_block_any) {
         break;   // mur
      }
      if(hbuf[y2] > 0)
      {
          p->data[i] = hbuf[y2];
          empty_count_local++;
      }
   }
   // vers le haut
   for (y2 = (int) y - 1; y2 >= 0; --y2) {
      int i = y2 * width + x;

      if (p->data[i] <= lusq_block_any) {
         break;   // mur
      }
      if(hbuf[y2] > 0)
      {
          p->data[i] = hbuf[y2];
          empty_count_local++;
      }
   }
   *empty_count = *empty_count + empty_count_local;
}





void puzzle_lights_off(lu_puzzle *p) {
   unsigned int x, y;

   if (p == NULL) {
      fprintf(stderr, "Cannot turn the lights off on an empty puzzle\n");
      return;
   }

   for (y = 0; y < p->height; ++y) {
      for (x = 0; x < p->width; ++x) {
         if (p->data[y * p->width + x] == lusq_enlighted) {
            p->data[y * p->width + x] = lusq_empty;
         }
      }
   }
}

unsigned int puzzle_count(const lu_puzzle *p, lu_square tp) {
   unsigned int i, count = 0;

   if (p == NULL) {
      return 0;
   }

   for (i = 0; i < p->width * p->height; ++i) {
      if (p->data[i] == tp) {
         ++count;
      }
   }

   return count;
}


void puzzle_count_1_2_3_wall_and_empty(const lu_puzzle *p,
        unsigned int * wall_number, unsigned int * empty_number) {
    unsigned int i, count_wall = 0, count_empty = 0;

        if (p == NULL) {
        return;
    }

    #pragma omp parallel for reduction(+:count_empty,count_wall)
    for (i = 0; i < p->width * p->height; ++i) 
    {
        count_empty += p->data[i] == lusq_empty;
        count_wall += (lusq_1 <= p->data[i]) && (p->data[i] <= lusq_3);

    }

    *wall_number=count_wall;
    *empty_number=count_empty;
}



unsigned int puzzle_check(const lu_puzzle *ref, lu_puzzle *sol) {

   if (sol == NULL) {
      fprintf(stderr, "Cannot check an empty solution\n");
      return 0;
   }

   if (ref == NULL) {
      fprintf(stderr, "Missing reference puzzle for comparison\n");
      return 0;
   }

   if (sol->width != ref->width || sol->height != ref->height) {
      fprintf(stderr, "The solution size does not match the reference puzzle ones\n");
      return 0;
   }

   unsigned int x, y, i;
   for (y = 0; y < sol->height; ++y) {
      for (x = 0; x < sol->width; ++x) {
         i = y * sol->width + x;

         // les murs doivent être identiques entre la solution et la référence
         if ((ref->data[i] <= lusq_block_any || sol->data[i] <= lusq_block_any)
             && sol->data[i] != ref->data[i])
         {
            fprintf(stderr, "Invalid or missing wall at %u,%u\n", x, y);
            puzzle_lights_off(sol);
            return 0;
         }

         // vérification des contraintes d'adjacence
         if (sol->data[i] <= lusq_4) {
            unsigned int lbcount = 0;

            if (x >= 1) {
               lbcount += sol->data[i - 1] == lusq_lbulb;
            }
            if (x < sol->width - 1) {
               lbcount += sol->data[i + 1] == lusq_lbulb;
            }
            if (y >= 1) {
               lbcount += sol->data[i - sol->width] == lusq_lbulb;
            }
            if (y < sol->height - 1) {
               lbcount += sol->data[i + sol->width] == lusq_lbulb;
            }

            if (sol->data[i] != lbcount) {
               fprintf(stderr,
                  "Invalid number of light bulbs around wall at %u,%u\n",
                  x,
                  y);
               puzzle_lights_off(sol);
               return 0;
            }

            continue;
         }

         // la suite ne concerne que les ampoules
         if (sol->data[i] != lusq_lbulb) {
            continue;
         }

         // éclairage et vérification des ampoules face à face
         int x2, y2, i2;

         // vers la gauche
         for (x2 = (int) x - 1; x2 >= 0; --x2) {
            i2 = y * sol->width + x2;

            if (sol->data[i2] == lusq_lbulb) {
               fprintf(stderr, "Two facing light bulbs at %u,%u and %d,%u\n",
                       x, y, x2, y);
               puzzle_lights_off(sol);
               return 0;
            }

            if (sol->data[i2] == lusq_empty || sol->data[i2] == lusq_enlighted) {
               sol->data[i2] = lusq_enlighted;
            } else {
               break;   // mur
            }
         }

         // vers la droite
         for (x2 = x + 1; (unsigned int) x2 < sol->width; ++x2) {
            i2 = y * sol->width + x2;

            if (sol->data[i2] == lusq_lbulb) {
               fprintf(stderr, "Two facing light bulbs at %u,%u and %d,%u\n",
                       x, y, x2, y);
               puzzle_lights_off(sol);
               return 0;
            }

            if (sol->data[i2] == lusq_empty || sol->data[i2] == lusq_enlighted) {
               sol->data[i2] = lusq_enlighted;
            } else {
               break;   // mur
            }
         }

         // vers le haut
         for (y2 = (int) y - 1; y2 >= 0; --y2) {
            i2 = y2 * sol->width + x;

            if (sol->data[i2] == lusq_lbulb) {
               fprintf(stderr, "Two facing light bulbs at %u,%u and %u,%d\n",
                       x, y, x, y2);
               puzzle_lights_off(sol);
               return 0;
            }

            if (sol->data[i2] == lusq_empty || sol->data[i2] == lusq_enlighted) {
               sol->data[i2] = lusq_enlighted;
            } else {
               break;   // mur
            }
         }

         // vers le bas
         for (y2 = y + 1; (unsigned int) y2 < sol->height; ++y2) {
            i2 = y2 * sol->width + x;

            if (sol->data[i2] == lusq_lbulb) {
               fprintf(stderr, "Two facing light bulbs at %u,%u and %u,%d\n",
                       x, y, x, y2);
               puzzle_lights_off(sol);
               return 0;
            }

            if (sol->data[i2] == lusq_empty || sol->data[i2] == lusq_enlighted) {
               sol->data[i2] = lusq_enlighted;
            } else {
               break;   // mur
            }
         }
      }
   }

   // recherche d'une case non éclairée
   for (y = 0; y < sol->height; ++y) {
      for (x = 0; x < sol->width; ++x) {
         if (sol->data[y * sol->width + x] == lusq_empty) {
            fprintf(stderr, "Square %u,%u is not enlightened\n", x, y);
            puzzle_lights_off(sol);
            return 0;
         }
      }
   }

   puzzle_lights_off(sol);

   return 1;
}

// hash fnv
unsigned int puzzle_hash(const lu_puzzle *p) {
   unsigned int res = 2166136261;
   unsigned int i;

   if (p == NULL) {
      return res;
   }

   for (i = 0; i < p->width * p->height; ++i) {
      unsigned int d = p->data[i] == lusq_enlighted ?
         (unsigned int) lusq_empty : (unsigned int) p->data[i];

      res = (res * 16777619) ^ d;
   }

   return res;
}

unsigned int puzzle_eq(const lu_puzzle *ref, const lu_puzzle *cmp) {
   // puzzle vides
   if (ref == NULL) {
      if (cmp == NULL) {
         return 1;
      }

      return 0;
   } else if (cmp == NULL) {
      return 0;
   }

   // les puzzle doivent avoir la même taille
   if (ref->height != cmp->height || ref->width != cmp->width) {
      return 0;
   }
   
   unsigned int i;
   for (i = 0; i < ref->width * ref->height; ++i) {
      unsigned int refd = ref->data[i] == lusq_enlighted ?
         (unsigned int) lusq_empty : (unsigned int) ref->data[i];
      unsigned int cmpd = cmp->data[i] == lusq_enlighted ?
         (unsigned int) lusq_empty : (unsigned int) cmp->data[i];

      if (refd != cmpd) {
         return 0;
      }
   }

   return 1;
}



__inline int wall_saturated(const lu_puzzle *p, unsigned int x,
   unsigned int y)
{
   unsigned int idx = y * p->width + x;

   // la case ne spécifie pas de contrainte particulière
   if ((p->data[idx] > lusq_4) || (x>=p->width) || (y>=p->height)) {
      return 0;
   }

   // compte les ampoules autour
   unsigned int lbcount = 0;
   if (x >= 1) {
      lbcount += p->data[idx - 1] == lusq_lbulb;
   }
   if (x < p->width - 1) {
      lbcount += p->data[idx + 1] == lusq_lbulb;
   }
   if (y >= 1) {
      lbcount += p->data[idx - p->width] == lusq_lbulb;
   }
   if (y < p->height - 1) {
      lbcount += p->data[idx + p->width] == lusq_lbulb;
   }

   return lbcount >= (unsigned int) p->data[idx];
}


__inline unsigned int wall_clear(const lu_puzzle *p, unsigned int x,
   unsigned int y)
{
   unsigned int idx = y * p->width + x;

   // pas une contrainte
   if (p->data[idx] > lusq_4) {
      return 1;
   }

   // compte les ampoules et les cases vides
   unsigned int lbcount = 0, ecount = 0;
   if (x >= 1) {
      ecount += p->data[idx - 1] == lusq_empty;
      lbcount += p->data[idx - 1] == lusq_lbulb;
   }
   if (x < p->width - 1) {
      ecount += p->data[idx + 1] == lusq_empty;
      lbcount += p->data[idx + 1] == lusq_lbulb;
   }
   if (y >= 1) {
      ecount += p->data[idx - p->width] == lusq_empty;
      lbcount += p->data[idx - p->width] == lusq_lbulb;
   }
   if (y < p->height - 1) {
      ecount += p->data[idx + p->width] == lusq_empty;
      lbcount += p->data[idx + p->width] == lusq_lbulb;
   }

   return ecount + lbcount >= (unsigned int) p->data[idx];
}
