#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lightup.h"

/*!
 * Lors de la résolution, on doit savoir quelles cases sont allumées sur la
 * ligne et sur la colonne d'une ampoule. Pour stocker cette info, on maintient
 * un pool de lignes et de colonnes pour limiter les allocations pendant la
 * résolution.
 * On a besoin d'une ligne et d'une colonne par ampoule, on en alloue et 
 * affecte un de chaque par requête.
 */
typedef struct {
   int *wbuf;              /*!< Buffers de lignes */
   int *hbuf;              /*!< Buffers de colonnes */
   unsigned int width;     /*!< Largeur du puzzle à résoudre */
   unsigned int height;    /*!< Hauteur du puzzle à résoudre */
   unsigned int nb_used;   /*!< Nombre de couples de buffers utilisés */
   unsigned int nb_allocs; /*!< Nombre de buffers alloués */
} wh_bufs;

/*!
 * Crée un nouveau pool de lignes et colonnes pour la résolution.
 *
 * \param width Largeur du puzzle à résoudre.
 * \param height Hauteur du puzzle à résoudre.
 * \return Un nouveau pool de buffers de lignes et colonnes pour la résolution
 * du puzzle.
 */
static wh_bufs *new_wh_bufs(unsigned int width, unsigned int height) {
   wh_bufs *res = (wh_bufs *) malloc(sizeof(*res));

   res->width = width;
   res->height = height;
   res->nb_used = 0;
   res->nb_allocs = width + height;    // estimation du nb d'ampoules

   res->wbuf = (int *) malloc(sizeof(*res->wbuf) * res->nb_allocs * width);
   res->hbuf = (int *) malloc(sizeof(*res->hbuf) * res->nb_allocs * height);

   return res;
}

/*!
 * Affecte une ligne et une colonne depuis le pool de buffers.
 *
 * \param bufs Le pool de lignes et colonnes.
 * \param[out] wbuf Ligne allouée dans le pool.
 * \param[out] hbuf Colonne allouée dans le pool.
 */
static void pop_wh_buf(wh_bufs *bufs, int **wbuf, int **hbuf) {
   // tous les buffers sont utilisés
   if (bufs->nb_used >= bufs->nb_allocs) {
      bufs->nb_allocs *= 2;

      bufs->wbuf = (int *) realloc(bufs->wbuf,
         sizeof(*bufs->wbuf) * bufs->width * bufs->nb_allocs);
      bufs->hbuf = (int *) realloc(bufs->hbuf,
         sizeof(*bufs->hbuf) * bufs->height * bufs->nb_allocs);
   }

   *wbuf = bufs->wbuf + bufs->width * bufs->nb_used;
   *hbuf = bufs->hbuf + bufs->height * bufs->nb_used;
   ++bufs->nb_used;
   
   memset(*wbuf, 0, sizeof(**wbuf) * bufs->width);
   memset(*hbuf, 0, sizeof(**hbuf) * bufs->height);
}

/*!
 * Libère la dernière ligne et la dernière colonne affectée dans le pool. Cette
 * ligne et cette colonne sont désormais réutilisables.
 *
 * \param bufs Le pool qui possède la dernière ligne et colonne affectées.
 */
static void release_wh_buf(wh_bufs *bufs) {
   if (bufs->nb_used == 0) {
      return;
   }

   --bufs->nb_used;
}

/*!
 * Libére la mémoire allouée par un pool de lignes et colonnes.
 *
 * \param bufs Le pool de buffers à libérer.
 */
static void free_wh_bufs(wh_bufs *bufs) {
   free(bufs->wbuf);
   free(bufs->hbuf);
   free(bufs);
}

/*!
 * Vérifie si la contrainte d'adjacence imposée par la case est respectée ou
 * pas.
 *
 * \param p Le puzzle qui contient la case.
 * \param x Abscisse de la case à vérifier.
 * \param y Ordonnée de la case à vérifier.
 * \return 1 s'il y a autant ou plus d'ampoules autour de la case que ce qui 
 * est imposé par la case. Renvoie 0 si la case n'est pas une case à contrainte
 * ou si il n'y a pas assez d'ampoule autour de la case.
 */
static __inline int wall_saturated(const lu_puzzle *p, unsigned int x,
   unsigned int y)
{
   unsigned int idx = y * p->width + x;

   // la case ne spécifie pas de contrainte particulière
   if (p->data[idx] > lusq_4) {
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

/*!
 * Vérifie s'il est possible de respecter la contrainte imposée par un mur,
 * c'est à dir qu'il y a déjà suffisament d'ampoules ou qu'il reste suffisament
 * de case vides autour.
 *
 * \param p Le puzzle qui contient la case à tester.
 * \param x Abscisse de la case à tester.
 * \param y Ordonnée de la case à tester.
 * \return 1 s'il reste assez de place pour placer assez d'ampoules autour de
 * la case contrainte ou si la case ne contient pas de contrainte.
 * Renvoie 0 si la contrainte ne plus assez de place autour
 * de la case pour respecter la contrainte.
 */
static __inline unsigned int wall_clear(const lu_puzzle *p, unsigned int x,
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

/*!
 * Résoud un puzzle light-up de façon récursive.
 *
 * \param[in,out] p Le puzzle à résoudre.
 * \param whbufs Pool de lignes et colonnes à utiliser pour la résolution.
 * \param nb_e Nombre de cases vides dans le puzzle.
 * \param forbiden Masque des cases à éviter. Doit contenir autant d'entrée qu'
 * il y a de cases dans \p p. Ne doit contenir que des 0 lors du premier appel.
 * \param[in, out] sol_id Numéro de la solution en cours
 * \param resdir Chemin vers le dossier de résultats
 *
 * \return 1 si le puzzle est résoluble, 0 sinon.
 */
static void solve(lu_puzzle *p, wh_bufs *whbufs, unsigned int nb_e,
   unsigned int *forbiden, unsigned int *sol_id, FILE *fd) 
{
   unsigned int x, y;

   unsigned int width = p->width;
   unsigned int height = p->height;

   // solution trouvée !
   if (nb_e == 0) {
      *sol_id = *sol_id + 1;

      puzzle_store(p, fd);
      return;
   }

   int *hbuf, *wbuf;
   pop_wh_buf(whbufs, &wbuf, &hbuf);

   // Itère sur les cases vides
   for (y = 0; y < p->height; ++y) {
      for (x = 0; x < p->width; ++x) {

         // la case n'est pas vide
         if (p->data[y * width + x] != lusq_empty) {
            continue;
         }

         // On doit éviter cette case car ça n'a pas abouti la dernière fois
         // qu'on a mis une ampoule à cet endroit
         if (forbiden[y * width + x]) {
            continue;
         }

         // si les murs autour de cette case interdisent la présence d'une
         // ampoule ici, on passe à la case suivante.
         if ((x >= 1 && wall_saturated(p, x - 1, y))
             || (x < width - 1 && wall_saturated(p, x + 1, y))
             || (y >= 1 && wall_saturated(p, x, y - 1))
             || (y < height - 1 && wall_saturated(p, x, y + 1)))
         {
            continue;
         }

         // éclaire la case en se souvenant des cases nouvellement allumées

         p->data[y * width + x] = lusq_lbulb;
         --nb_e;
   
         int x2, y2;

         // on éclaire à gauche
         for (x2 = (int) x - 1; x2 >= 0; --x2) {
            int i = y * width + x2;

            if (p->data[i] == lusq_enlighted) {
               continue;
            }

            if (p->data[i] != lusq_empty) {
               break;   // mur
            }

            p->data[i] = lusq_enlighted;
            wbuf[x2] = 1;
            --nb_e;            
         }

         // à droite
         for (x2 = x + 1; (unsigned int) x2 < width; ++x2) {
            int i = y * width + x2;

            if (p->data[i] == lusq_enlighted) {
               continue;
            }

            if (p->data[i] != lusq_empty) {
               break;   // mur
            }

            p->data[i] = lusq_enlighted;
            wbuf[x2] = 1;
            --nb_e;
         }

         // en haut
         for (y2 = (int) y - 1; y2 >= 0; --y2) {
            int i = y2 * width + x;

            if (p->data[i] == lusq_enlighted) {
               continue;
            }

            if (p->data[i] != lusq_empty) {
               break; // mur
            }

            p->data[i] = lusq_enlighted;
            hbuf[y2] = 1;
            --nb_e;            
         }

         // en bas
         for (y2 = y + 1; (unsigned int) y2 < height; ++y2) {
            int i = y2 * width + x;

            if (p->data[i] == lusq_enlighted) {
               continue;
            }

            if (p->data[i] != lusq_empty) {
               break; // mur
            }

            p->data[i] = lusq_enlighted;
            hbuf[y2] = 1;
            --nb_e;
         }

         // vérifie que les cases qu'on vient d'allumer ne contredisent pas des
         // contraintes encore non respectées
         unsigned int minx, maxx, miny, maxy, wallsok = 1;
         
         minx = (x >= 1) ? x - 1 : 0;
         maxx = (x < width - 1) ? x + 1 : width - 1;
         miny = (y >= 1) ? y - 1 : 0;
         maxy = (y < height - 1) ? y + 1 : height - 1;

         for (y2 = 0; (unsigned int) y2 < height && wallsok; ++y2) {
            for (x2 = minx; (unsigned int) x2 <= maxx && wallsok; ++x2) {
               wallsok &= wall_clear(p, x2, y2);
            }
         }
         for (y2 = miny; (unsigned int) y2 <= maxy && wallsok; ++y2) {
            for (x2 = 0; (unsigned int) x2 < width && wallsok; ++x2) {
               wallsok &= wall_clear(p, x2, y2);
            }
         }

         // si une solution est possible, on continue la résolution
         if (wallsok) {
            solve(p, whbufs, nb_e, forbiden, sol_id, fd);
         }

         // on retire notre ampoule
         p->data[y * width + x] = lusq_empty;
         ++nb_e;

         // Il faut éviter de placer une ampoule ici dans les prochaines
         // itérations: on a tout testé avec cette configuration
         forbiden[y * width + x] = 1;

         // Les appels récursifs ont peut être interdit certaines cases qui
         // doivent être re-testées une fois l'ampoule en cours retirée
         unsigned int fidx; 
         for (fidx = y * width + x + 1; fidx < width * height; ++fidx) {
            forbiden[fidx] = 0;
         }

         // on met à jour l'information d'éclairage à gauche
         for (x2 = (int) x - 1; x2 >= 0; --x2) {
            if (wbuf[x2]) {
               int i = y * width + x2;

               p->data[i] = lusq_empty;
               wbuf[x2] = 0;
               ++nb_e;
            }
         }

         // à droite
         for (x2 = x + 1; (unsigned int) x2 < width; ++x2) {
            if (wbuf[x2]) {
               int i = y * width + x2;

               p->data[i] = lusq_empty;
               wbuf[x2] = 0;
               ++nb_e;
            }
         }

         // en haut
         for (y2 = (int) y - 1; y2 >= 0; --y2) {
            if (hbuf[y2]) {
               int i = y2 * width + x;

               p->data[i] = lusq_empty;
               hbuf[y2] = 0;
               ++nb_e;
            }
         }

         // en bas
         for (y2 = y + 1; (unsigned int) y2 < height; ++y2) {
            if (hbuf[y2]) {
               int i = y2 * width + x;

               p->data[i] = lusq_empty;
               hbuf[y2] = 0;
               ++nb_e;
            }
         }
      }
   }

   // on n'a plus besoin de ces buffers
   release_wh_buf(whbufs);
}

int solver_main(int argc, char **argv) {
   if (argc < 3) {
      printf("Solves a ligthup puzzle\n");
      printf("Usage: %s <problem> <output_file>\n", argv[0]);
      return EXIT_FAILURE;
   }

   printf("Loading %s for solving...\n", argv[1]);
   lu_puzzle *p = puzzle_load(argv[1], 1);

   FILE *fd = puzzle_open_storage_file(argv[2], p->width, p->height);

   wh_bufs *whbufs = new_wh_bufs(p->width, p->height);

   printf("Solving...\n");

   unsigned int nb_e = puzzle_count(p, lusq_empty);
   printf("Problem size = %u\n", nb_e);

   unsigned int *forbiden = (unsigned int *)malloc(sizeof(*forbiden) * p->width * p->height);
   memset(forbiden, 0, sizeof(*forbiden) * p->width * p->height);

   unsigned int sol_id = 0;

   solve(p, whbufs, nb_e, forbiden, &sol_id, fd);
   printf("Found %u solutions\n", sol_id);

   free(forbiden);
   free_wh_bufs(whbufs);
   puzzle_destroy(p);
   fclose(fd);

   return EXIT_SUCCESS;
}

#if !defined (__BUILD_FOR_USE_WITH_ISDA__) || defined (__BUILD_FOR_USE_WITH_ISDA_AS_STANDALONE_TOOL__)

int main(int argc, char **argv) {
   return solver_main(argc, argv);
}

#endif

