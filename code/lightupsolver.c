#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lightup.h"
#include "lightupsolver.h"
#include "utils.h"

position_array left_, right_ ,top_ ,bottom_,center_;

/*!
 * Lors de la résolution, on doit savoir quelles cases sont allumées sur la
 * ligne et sur la colonne d'une ampoule. Pour stocker cette info, on maintient
 * un pool de lignes et de colonnes pour limiter les allocations pendant la
 * résolution.
 * On a besoin d'une ligne et d'une colonne par ampoule, on en alloue et 
 * affecte un de chaque par requête.
 */
typedef struct {
   char *wbuf;              /*!< Buffers de lignes */
   char *hbuf;              /*!< Buffers de colonnes */
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
wh_bufs *new_wh_bufs(unsigned int width, unsigned int height, unsigned int size) {
   wh_bufs *res = (wh_bufs *) malloc(sizeof(*res));

   res->width = width;
   res->height = height;
   res->nb_used = 0;
   res->nb_allocs = (size > 0) ? size : width + height;    // estimation du nb d'ampoules

   res->wbuf = (char *) malloc(sizeof(*res->wbuf) * res->nb_allocs * width);
   res->hbuf = (char *) malloc(sizeof(*res->hbuf) * res->nb_allocs * height);

   return res;
}

/*!
 * Affecte une ligne et une colonne depuis le pool de buffers.
 *
 * \param bufs Le pool de lignes et colonnes.
 * \param[out] wbuf Ligne allouée dans le pool.
 * \param[out] hbuf Colonne allouée dans le pool.
 */
void pop_wh_buf(wh_bufs *bufs, char **wbuf, char **hbuf) {
   // tous les buffers sont utilisés
   if (bufs->nb_used >= bufs->nb_allocs) {
      bufs->nb_allocs *= 2;

      bufs->wbuf = (char *) realloc(
              bufs->wbuf,
              sizeof(*bufs->wbuf) * bufs->width * bufs->nb_allocs);
      bufs->hbuf = (char *) realloc(
              bufs->hbuf,
              sizeof(*bufs->hbuf) * bufs->height * bufs->nb_allocs);
   }

   *wbuf = bufs->wbuf;
   bufs->wbuf += bufs->width;
   *hbuf = bufs->hbuf;
   bufs->hbuf += bufs->height;
   ++bufs->nb_used;
   
   memset(*wbuf, 0, sizeof(**wbuf) * bufs->width);
   memset(*hbuf, 0, sizeof(**hbuf) * bufs->height);
}

void get_head_wh_buf(wh_bufs *bufs, char **wbuf, char **hbuf)
{
   if(bufs->nb_used == 0)
   {
       *wbuf = NULL;
       *hbuf = NULL;
   }
   else
   {
       *wbuf = bufs->wbuf - bufs->width;
       *hbuf = bufs->hbuf - bufs->height;
   }
}

/*!
 * Libère la dernière ligne et la dernière colonne affectée dans le pool. Cette
 * ligne et cette colonne sont désormais réutilisables.
 *
 * \param bufs Le pool qui possède la dernière ligne et colonne affectées.
 */
void release_wh_buf(wh_bufs *bufs) {
   if (bufs->nb_used == 0) {
      return;
   }

    bufs->wbuf = bufs->wbuf - bufs->width;
    bufs->hbuf = bufs->hbuf - bufs->height;

   --bufs->nb_used;
}

/*!
 * Libére la mémoire allouée par un pool de lignes et colonnes.
 *
 * \param bufs Le pool de buffers à libérer.
 */
void free_wh_bufs(wh_bufs *bufs) {
   free(bufs->wbuf - bufs->nb_used*bufs->width);
   free(bufs->hbuf - bufs->nb_used*bufs->height);
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


void print_solutions(position_array pa_empty, int * solutions)
{
    unsigned int i = 0;
    while(solutions[i] != -2)
    {
        printf("[");
        while(solutions[i] != -1)
        {
            printf(" (%u,%u),", pa_empty.array[solutions[i]].line, pa_empty.array[solutions[i]].column);
            i++;
        }
        printf("]\n");
        i++;
    }
}

void print_solution(position_array pa_empty, int * solutions)
{
    unsigned int i = 0;
    printf("[");
    while(solutions[i] != -1)
    {
        printf(" (%u,%u),", pa_empty.array[solutions[i]].line, pa_empty.array[solutions[i]].column);
        i++;
    }
    printf("]\n");
}


char wall_heuristic(lu_puzzle *p,
        position_array * pa_left_border,
        position_array * pa_right_border,
        position_array * pa_top_border,
        position_array * pa_bottom_border,
        position_array * pa_center)
{
    unsigned int c = 0,
                 l = 0,
                 count = 0,
                 current_state = 0,
                 index = 0;

    unsigned char change = 0;
    
    unsigned int width = p->width;
    unsigned int height = p->height;

    position_array pa_local_center = *pa_center,
                   pa_local_left_border = *pa_left_border,
                   pa_local_right_border = *pa_right_border,
                   pa_local_top_border = *pa_top_border,
                   pa_local_bottom_border = *pa_bottom_border;

    //COINS
    //Coin haut gauche
    current_state = p->data[0];
    if(current_state == lusq_0)
    {
        if(p->data[1] == lusq_empty)
        {
            p->data[1] = lusq_impossible;
            change = 1;
        }
        if(p->data[width] == lusq_empty)
        {
            p->data[width] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[1] == lusq_empty) +
            (p->data[width] == lusq_empty) +
            (p->data[1] == lusq_lbulb) +
            (p->data[width] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[1] == lusq_empty)
            {
                puzzle_light_on(p,1,0);
                change = 1;
            }
            if(p->data[width] == lusq_empty)
            {
                puzzle_light_on(p,0,1);
                change = 1;
            }
        }
    }
    //Coin haut droit
    current_state = p->data[width-1];
    if(current_state == lusq_0)
    {
        if(p->data[width-2] == lusq_empty)
        {
            p->data[width-2] = lusq_impossible;
            change = 1;
        }
        if(p->data[2*width - 1] == lusq_empty)
        {
            p->data[2*width - 1] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[width-2] == lusq_empty) +
            (p->data[2*width - 1] == lusq_empty) +
            (p->data[width-2] == lusq_lbulb) +
            (p->data[2*width - 1] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[width-2] == lusq_empty) 
            {
                puzzle_light_on(p,width-2,0);
                change = 1;
            }
            if(p->data[2*width - 1] == lusq_empty)
            {
                puzzle_light_on(p,width-1,1);
                change = 1;
            }
        }
    }
    //Coin bas gauche
    current_state = p->data[width*(height-1)];
    if(current_state == lusq_0)
    {
        if(p->data[width*(height-2)] == lusq_empty)
        {
            p->data[width*(height-2)] = lusq_impossible;
            change = 1;
        }
        if(p->data[width*(height-1) + 1] == lusq_empty)
        {
            p->data[width*(height-1) + 1] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[width*(height-2)] == lusq_empty) +
            (p->data[width*(height-1) + 1] == lusq_empty) +
            (p->data[width*(height-2)] == lusq_lbulb) +
            (p->data[width*(height-1) + 1] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[width*(height-2)] == lusq_empty)
            {
                puzzle_light_on(p,0,height-2);
                change = 1;
            }
            if(p->data[width*(height-1) + 1] == lusq_empty)
            {
                puzzle_light_on(p,1,height-1);
                change = 1;
            }
        }
    }
    //Coin bas droit
    current_state = p->data[width*height - 1];
    if(current_state == lusq_0)
    {
        if(p->data[width*height - 2] == lusq_empty)
        {
            p->data[width*height - 2] = lusq_impossible;
            change = 1;
        }
        if(p->data[width*(height-1) - 1] == lusq_empty)
        {
            p->data[width*(height-1) - 1] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[width*height - 2] == lusq_empty) +
            (p->data[width*(height-1) - 1] == lusq_empty) +
            (p->data[width*height - 2] == lusq_lbulb) +
            (p->data[width*(height-1) - 1] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[width*height - 2] == lusq_empty)
            {
                puzzle_light_on(p,width-2,height-1);
                change = 1;
            }
            if(p->data[width*(height-1) - 1] == lusq_empty)
            {
                puzzle_light_on(p,width-1,height-2);
                change = 1;
            }
        }
    }

    //BORD SANS LES COINS/////////
    //BORD GAUCHE ET DROIT
    for (index = 0; index < pa_local_left_border.size ; ++index)
    {
        l = pa_local_left_border.array[index].line;
        current_state = p->data[l * width];
        if(current_state == lusq_0 || wall_saturated(p,0,l))
        {
            if(p->data[(l-1) * width] == lusq_empty) p->data[(l-1) * width] = lusq_impossible;
            if(p->data[(l+1) * width] == lusq_empty) p->data[(l+1) * width] = lusq_impossible;
            if(p->data[l * width + 1] == lusq_empty) p->data[l * width + 1] = lusq_impossible;
            remove_from_position_array(&pa_local_left_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[(l-1) * width] == lusq_empty) +
                (p->data[(l+1) * width] == lusq_empty) +
                (p->data[l * width + 1] == lusq_empty) +
                (p->data[(l-1) * width] == lusq_lbulb) +
                (p->data[(l+1) * width] == lusq_lbulb) +
                (p->data[l * width + 1] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(l-1) * width] == lusq_empty) puzzle_light_on(p,0,l-1);
                if(p->data[(l+1) * width] == lusq_empty) puzzle_light_on(p,0,l+1);
                if(p->data[l * width + 1] == lusq_empty) puzzle_light_on(p,1,l);
                remove_from_position_array(&pa_local_left_border,index);
                index--;
                change = 1;
            }
        }
    }
    for (index = 0; index < pa_local_right_border.size ; ++index)
    {
        l = pa_local_right_border.array[index].line;
        current_state = p->data[l * width + width -1];
        if(current_state == lusq_0 || wall_saturated(p,width-1,l))
        {
            if(p->data[(l-1) * width + width-1] == lusq_empty) p->data[(l-1) * width + width-1] = lusq_impossible;
            if(p->data[(l+1) * width + width-1] == lusq_empty) p->data[(l+1) * width + width-1] = lusq_impossible;
            if(p->data[l * width + width-2] == lusq_empty) p->data[l * width + width-2] = lusq_impossible;
            remove_from_position_array(&pa_local_right_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[(l-1) * width + width-1] == lusq_empty) +
                (p->data[(l+1) * width + width-1] == lusq_empty) +
                (p->data[l * width + width-2] == lusq_empty) +
                (p->data[(l-1) * width + width-1] == lusq_lbulb) +
                (p->data[(l+1) * width + width-1] == lusq_lbulb) +
                (p->data[l * width + width-2] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(l-1) * width + width-1] == lusq_empty) puzzle_light_on(p,width-1,l-1);
                if(p->data[(l+1) * width + width-1] == lusq_empty) puzzle_light_on(p,width-1,l+1);
                if(p->data[l * width + width-2] == lusq_empty) puzzle_light_on(p,width-2,l);
                remove_from_position_array(&pa_local_right_border,index);
                index--;
                change = 1;
            }
        }
    }
    //BORD HAUT ET BAS
    for (index = 0; index < pa_local_top_border.size ; ++index)
    {
        c = pa_local_top_border.array[index].column;
        current_state = p->data[c];
        if(current_state == lusq_0 || wall_saturated(p,c,0))
        {
            if(p->data[c-1] == lusq_empty) p->data[c-1] = lusq_impossible;
            if(p->data[c+1] == lusq_empty) p->data[c+1] = lusq_impossible;
            if(p->data[width + c] == lusq_empty) p->data[width + c] = lusq_impossible;
            remove_from_position_array(&pa_local_top_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[c-1] == lusq_empty) +
                (p->data[c+1] == lusq_empty) +
                (p->data[width + c] == lusq_empty) +
                (p->data[c-1] == lusq_lbulb) +
                (p->data[c+1] == lusq_lbulb) +
                (p->data[width + c] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[c-1] == lusq_empty) puzzle_light_on(p,c-1,0);
                if(p->data[c+1] == lusq_empty) puzzle_light_on(p,c+1,0);
                if(p->data[width + c] == lusq_empty) puzzle_light_on(p,c,1);
                remove_from_position_array(&pa_local_top_border,index);
                index--;
                change = 1;
            }
        }
    }
    for (index = 0; index < pa_local_bottom_border.size ; ++index)
    {
        c = pa_local_bottom_border.array[index].column;
        current_state = p->data[(height-1)*width + c];
        if(current_state == lusq_0 || wall_saturated(p,c,height-1))
        {
            if(p->data[(height-1)*width + c-1] == lusq_empty) p->data[(height-1)*width + c-1] = lusq_impossible;
            if(p->data[(height-1)*width + c+1] == lusq_empty) p->data[(height-1)*width + c+1] = lusq_impossible;
            if(p->data[(height-2)*width + c] == lusq_empty) p->data[(height-2)*width +  + c] = lusq_impossible;
            remove_from_position_array(&pa_local_bottom_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[(height-1)*width + c-1] == lusq_empty) +
                (p->data[(height-1)*width + c+1] == lusq_empty) +
                (p->data[(height-2)*width + c] == lusq_empty) +
                (p->data[(height-1)*width + c-1] == lusq_lbulb) +
                (p->data[(height-1)*width + c+1] == lusq_lbulb) +
                (p->data[(height-2)*width + c] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(height-1)*width + c-1] == lusq_empty) puzzle_light_on(p,c-1,height-1);
                if(p->data[(height-1)*width + c+1] == lusq_empty) puzzle_light_on(p,c+1,height-1);
                if(p->data[(height-2)*width + c] == lusq_empty) puzzle_light_on(p,c,height-2);
                remove_from_position_array(&pa_local_bottom_border,index);
                index--;
                change = 1;
            }
        }
    }

    //INTERIEUR SANS BORD NI COINS
    for (index = 0; index < pa_local_center.size ; ++index)
    {
        l = pa_local_center.array[index].line;
        c = pa_local_center.array[index].column;
        current_state = p->data[l * width + c];
        if(current_state == lusq_0 || wall_saturated(p,c,l))
        {
            if(p->data[(l-1) * width + c] == lusq_empty) p->data[(l-1) * width + c] = lusq_impossible;
            if(p->data[(l+1) * width + c] == lusq_empty) p->data[(l+1) * width + c] = lusq_impossible;
            if(p->data[l * width + c - 1] == lusq_empty) p->data[l * width + c - 1] = lusq_impossible;
            if(p->data[l * width + c + 1] == lusq_empty) p->data[l * width + c + 1] = lusq_impossible;
            remove_from_position_array(&pa_local_center,index);
            change = 1;
        }
        else if(current_state <= lusq_4)
        {
            count = 
                (p->data[(l-1) * width + c] == lusq_empty) +
                (p->data[(l+1) * width + c] == lusq_empty) +
                (p->data[l * width + c - 1] == lusq_empty) +
                (p->data[l * width + c + 1] == lusq_empty) +
                (p->data[(l-1) * width + c] == lusq_lbulb) +
                (p->data[(l+1) * width + c] == lusq_lbulb) +
                (p->data[l * width + c - 1] == lusq_lbulb) +
                (p->data[l * width + c + 1] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(l-1) * width + c] == lusq_empty) puzzle_light_on(p,c,l-1);
                if(p->data[(l+1) * width + c] == lusq_empty) puzzle_light_on(p,c,l+1);
                if(p->data[l * width + c - 1] == lusq_empty) puzzle_light_on(p,c-1,l);
                if(p->data[l * width + c + 1] == lusq_empty) puzzle_light_on(p,c+1,l);
                remove_from_position_array(&pa_local_center,index);
                index--;
                change = 1;
            }
        }
    }

    
    *pa_center = pa_local_center;
    *pa_left_border = pa_local_left_border;
    *pa_right_border = pa_local_right_border;
    *pa_top_border = pa_local_top_border; 
    *pa_bottom_border = pa_local_bottom_border;

    return change;
}


char empty_and_impossible_heuristic(lu_puzzle * p, position_array * pa_empty, position_array * pa_impossible)
{
    unsigned int c = 0,
                 l = 0,
                 index = 0;

    unsigned char change = 0;
    
    unsigned int width = p->width;

    position_array pa_local_empty = *pa_empty,
                   pa_local_impossible = *pa_impossible;


    position lb_pos = {0,0};
    for(index = 0; index < pa_local_impossible.size ; index++)
    {
        l = pa_local_impossible.array[index].line;
        c = pa_local_impossible.array[index].column;
    
        if(p->data[l * width + c] != lusq_impossible)
        {
            remove_from_position_array(&pa_local_impossible, index);
            index--;
        }
        else if(number_of_lightbulb_possible(p, l, c, &lb_pos) == 1)
        {
            puzzle_light_on(p, lb_pos.column, lb_pos.line);
            remove_from_position_array(&pa_local_impossible, index);
            index--;
            change = 1;
        }
    }
    //Essaye de remplir les cases vides qui doivent s'éclairer eux même (lorsqu'aucune case n'est libre autour d'eux pour l'éclairer).
    for(index = 0; index < pa_local_empty.size ; index++)
    {
        l = pa_local_empty.array[index].line;
        c = pa_local_empty.array[index].column;
    
        if(p->data[l * width + c] != lusq_empty)
        {
            if(p->data[l * width + c] == lusq_impossible)
            {
                add_to_position_array(&pa_local_impossible, pa_local_empty.array[index]);
                change = 1;
            }
            remove_from_position_array(&pa_local_empty, index);
            index--;
        }
        else if(is_alone(p, l, c) == 1)
        {
            puzzle_light_on(p, pa_local_empty.array[index].column, pa_local_empty.array[index].line);
            remove_from_position_array(&pa_local_empty, index);
            index--;
            change = 1;
        }
    }

    *pa_empty = pa_local_empty;
    *pa_impossible = pa_local_impossible;

    return change;
}

void pre_solve(lu_puzzle *p, position_array * positions_empty,
        position_array * positions_impossible,position_array * left,
        position_array * right, position_array * top, position_array * bottom,
        position_array * center)
{
    unsigned int c = 0,
                 l = 0,
                 index = 0;

    unsigned int change = 0;
    
    unsigned int width = p->width;
    unsigned int height = p->height;

    
    position_array pa_empty = new_position_array();
    position_array pa_impossible = new_position_array();

    position_array pa_center = new_position_array_with_size((width-2)*(height-2));
    position_array pa_left_border = new_position_array_with_size(height);
    position_array pa_right_border = new_position_array_with_size(height);
    position_array pa_top_border = new_position_array_with_size(width);
    position_array pa_bottom_border = new_position_array_with_size(width);



    //Filling the array of wall with number on it
    index = 0;
    for (l = 1; l < p->height-1; ++l)
    {
        for (c = 1; c < p->width -1; ++c)
        {
            pa_center.array[index] = (position){l,c};
            index += (p->data[l * width + c] <= lusq_4);
        }
    }
    pa_center.size = index;

    index = 0;
    for (l = 1; l < height-1; ++l)
    {
        pa_left_border.array[index] = (position){l,0};
        index += (p->data[l * width] <= lusq_4);
    }
    pa_left_border.size = index;
    index = 0;
    for (l = 1; l < height-1; ++l)
    {
        pa_right_border.array[index] = (position){l,width-1};
        index += (p->data[l * width + width -1] <= lusq_4);
    }
    pa_right_border.size = index;
    index = 0;
    for (c = 1; c < width-1; ++c)
    {
        pa_top_border.array[index] = (position){0,c};
        index += (p->data[c] <= lusq_4);
    }
    pa_top_border.size = index;
    index = 0;
    for (c = 1; c < width-1; ++c)
    {
        pa_bottom_border.array[index] = (position){height-1,c};
        index += (p->data[(height-1)*width + c] <= lusq_4);
    }
    pa_bottom_border.size = index;

    /////////////////////////////////////////
    //APPEL A LA FONCTION HEURISTIC QUI REMPLIE LES CASES VIDES SELON LES CONTRAINTES DES MURS
    /////////////////////////////////////////
    do
    {
        change = wall_heuristic(
                p, 
                &pa_left_border,
                &pa_right_border,
                &pa_top_border,
                &pa_bottom_border,
                &pa_center);
    }
    while(change != 0);


    unsigned index_empty = 0, index_impossible = 0;
    for (l = 0; l < p->height; ++l)
    {
        for (c = 0; c < p->width; ++c)
        {
            pa_empty.array[index_empty] = (position){l,c};
            pa_impossible.array[index_impossible] = (position){l,c};
            index_empty += (p->data[l * width + c] == lusq_empty);
            index_impossible +=(p->data[l * width + c] == lusq_impossible);
        }
    }
    pa_empty.size = index_empty;
    pa_impossible.size = index_impossible;

    
    do
    {
        change = 0;

        change += empty_and_impossible_heuristic(
                p, 
                &pa_empty,
                &pa_impossible);

        /////////////////////////////////////////
        //APPEL A LA FONCTION HEURISTIC QUI REMPLIE LES CASES VIDES SELON LES CONTRAINTES DES MURS
        /////////////////////////////////////////
        change += wall_heuristic(
                p, 
                &pa_left_border,
                &pa_right_border,
                &pa_top_border,
                &pa_bottom_border,
                &pa_center);
    }
    while(change != 0);




    *positions_empty = pa_empty;
    *positions_impossible = pa_impossible;
    *left = pa_left_border;
    *right = pa_right_border;
    *top = pa_top_border;
    *bottom = pa_bottom_border;
    *center = pa_center;
}

void print_class(position_array* pa_array, unsigned int pa_array_size)
{
    unsigned int index_array, index;
    for(index_array = 0 ; index_array < pa_array_size; index_array++)
    {
        for(index = 0; index < pa_array[index_array].size ; index++)
        {
            printf("(%u,%u) | ",pa_array[index_array].array[index].line ,pa_array[index_array].array[index].column);
        }
        printf(".\n");
    }
}


void classify_positions(lu_puzzle *p, position_array ** pa_array_empty, position_array ** pa_array_impossible, unsigned int * pa_array_size, position_array pa_empty)
{
    position_array * pa_local_array_empty = NULL, *pa_local_array_impossible = NULL;
    unsigned int pa_local_array_size = 0;

    int width = p->width;
    int height = p->height;

    unsigned int index_empty = 0, index = 0;
    int c = 0, l = 0;

    position_array current_pa_empty = {0,0,0};
    position_array current_pa_impossible = {0,0,0};
    unsigned int total_empty_size = 0;
    position_array pa_flood = new_position_array_with_size(pa_empty.size);
    position current_pos, flood_pos;

    pa_local_array_empty = (position_array*) malloc(sizeof(position_array) * pa_empty.size);
    pa_local_array_impossible = (position_array*) malloc(sizeof(position_array) * pa_empty.size);

    for(index_empty = 0; index_empty < pa_empty.size ; index_empty++)
    {
        current_pos = pa_empty.array[index_empty];
        if(p->data[current_pos.line*width + current_pos.column] == lusq_flood || p->data[current_pos.line*width + current_pos.column] == lusq_flood_impossible) continue;//Déjà ajouté à une classe

        current_pa_empty = new_position_array_with_size(pa_empty.size - total_empty_size);//Taille dégressive
        current_pa_impossible = new_position_array();
        add_to_position_array(&pa_flood,current_pos);

        while(pa_flood.size > 0)
        {
            flood_pos = pa_flood.array[pa_flood.size-1];
            pa_flood.size--;

            if(p->data[flood_pos.line*width + flood_pos.column] == lusq_empty)
            {
                p->data[flood_pos.line*width + flood_pos.column] = lusq_flood;
                add_to_position_array(&current_pa_empty, flood_pos);
            }
            else if(p->data[flood_pos.line*width + flood_pos.column] == lusq_impossible)
            {
                p->data[flood_pos.line*width + flood_pos.column] = lusq_flood_impossible;
                add_to_position_array(&current_pa_impossible, flood_pos);
            }
            else continue;
            
            c = flood_pos.column + 1;
            while((c < width) && (p->data[flood_pos.line*width + c] == lusq_enlighted)) c++;
            if((c < width) && ((p->data[flood_pos.line*width + c] == lusq_empty) || (p->data[flood_pos.line*width + c] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){flood_pos.line,c});
            }

            c = flood_pos.column - 1;
            while((c >= 0) && (p->data[flood_pos.line*width + c] == lusq_enlighted)) c--;
            if((c >= 0) && ((p->data[flood_pos.line*width + c] == lusq_empty) || (p->data[flood_pos.line*width + c] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){flood_pos.line,c});
            }

            l = flood_pos.line + 1;
            while((l < height) && (p->data[l*width + flood_pos.column] == lusq_enlighted)) l++;
            if((l < height) && ((p->data[l*width + flood_pos.column] == lusq_empty) || (p->data[l*width + flood_pos.column] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){l,flood_pos.column});
            }

            l = flood_pos.line - 1;
            while((l >= 0) && (p->data[l*width + flood_pos.column] == lusq_enlighted)) l--;
            if((l >= 0) && ((p->data[l*width + flood_pos.column] == lusq_empty) || (p->data[l*width + flood_pos.column] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){l,flood_pos.column});
            }
        }
        pa_local_array_empty[pa_local_array_size] = current_pa_empty;
        pa_local_array_impossible[pa_local_array_size] = current_pa_impossible;
        pa_local_array_size++;
        total_empty_size += current_pa_empty.size;
    }

    unsigned int i = 0;
    for(i = 0; i< pa_local_array_size; i++)
    {
        for(index = 0; index < pa_local_array_empty[i].size ; index++)
        {
            current_pos = pa_local_array_empty[i].array[index];
            p->data[current_pos.line*width + current_pos.column] = lusq_empty;
        }

        for(index = 0; index < pa_local_array_impossible[i].size ; index++)
        {
            current_pos = pa_local_array_impossible[i].array[index];
            p->data[current_pos.line*width + current_pos.column] = lusq_impossible;
        }
    }

    *pa_array_empty = pa_local_array_empty;
    *pa_array_impossible = pa_local_array_impossible;
    *pa_array_size = pa_local_array_size;

    return;
}

//Regarde s'il est possible d'allumer cette emplacement (si l'emplacement est déjà allumé, ou si il y a conflit avec un mur à coté).
int impossible_to_light(lu_puzzle *p, position pos)
{
    char boolean = 0;
    boolean = (p->data[pos.line * p->width + pos.column] != lusq_empty)
    || wall_saturated(p,pos.column - 1, pos.line)
    || wall_saturated(p,pos.column + 1, pos.line)
    || wall_saturated(p,pos.column, pos.line-1)
    || wall_saturated(p,pos.column, pos.line+1);

    return boolean;
}

int possible_to_light(lu_puzzle *p, position pos)
{
    char boolean = 0;
    boolean = (p->data[pos.line * p->width + pos.column] == lusq_empty)
    && !wall_saturated(p,pos.column - 1, pos.line)
    && !wall_saturated(p,pos.column + 1, pos.line)
    && !wall_saturated(p,pos.column, pos.line-1)
    && !wall_saturated(p,pos.column, pos.line+1);

    return boolean;
}


//Regarde si la solution allume tout les emplacements vide (possiblement impossible) que represente pa_empty
int solution_is_complete(lu_puzzle *p, position_array pa_empty, position_array pa_impossible)
{
    unsigned int index = 0;
    unsigned int return_boolean = 0;

    index = 0;
    while((index < pa_empty.size) &&
          ((p->data[pa_empty.array[index].line*p->width + pa_empty.array[index].column] == lusq_lbulb) ||
           (p->data[pa_empty.array[index].line*p->width + pa_empty.array[index].column] == lusq_enlighted))
         ) index++;
    if(index >= pa_empty.size)
    {
        index = 0;
        while((index < pa_impossible.size) && (p->data[pa_impossible.array[index].line*p->width + pa_impossible.array[index].column] == lusq_enlighted)) index++;
        return_boolean = index >= pa_impossible.size;
    }

    return return_boolean;
}


/*!
 * Résoud un puzzle light-up de façon itérative.
 */
void solve(lu_puzzle *p, position_array pa_empty, position_array pa_impossible, int_array * solutions, unsigned int *sol_id) 
{
    unsigned int index = 0, sol_id_local = 0, useless = 0;
    wh_bufs * whbufs = new_wh_bufs(p->width, p->height, pa_empty.size);
    char *wbuf = NULL, *hbuf = NULL;
    int empty_count = pa_empty.size + pa_impossible.size;

    int_array ia_current_solution = new_int_array();
    *solutions = new_int_array();
    do
    {
        useless++;  //FIXME
        while((index < pa_empty.size) && impossible_to_light(p, pa_empty.array[index])) index++;
        if(index < pa_empty.size)
        {
            pop_wh_buf(whbufs, &wbuf, &hbuf);
            puzzle_light_on_with_bufs(p, pa_empty.array[index].column, pa_empty.array[index].line, wbuf, hbuf, &empty_count);
            add_to_int_array(&ia_current_solution, index);
        }
        else
        {
            //if(solution_is_complete(p, pa_empty, pa_impossible) == 1)
            if(empty_count <= 0)
            {
                if((solutions->size + ia_current_solution.size + 1) > solutions->max_size)
                {
                    solutions->max_size *= 2;
                    solutions->array = (int*)realloc(solutions->array, sizeof(int) * solutions->max_size);
                }
                memcpy(&solutions->array[solutions->size], ia_current_solution.array, sizeof(*ia_current_solution.array) * ia_current_solution.size);
                solutions->size += ia_current_solution.size;
                add_to_int_array(solutions, -1);
                sol_id_local++;
            }
            
            --ia_current_solution.size;
            index = ia_current_solution.array[ia_current_solution.size];//Dépile le dernier élément qui ne sert à rien
            get_head_wh_buf(whbufs, &wbuf, &hbuf);
            puzzle_light_off_with_bufs(p, pa_empty.array[index].column, pa_empty.array[index].line, wbuf, hbuf, &empty_count);
            release_wh_buf(whbufs);
        }
        index++;
    }
    while((ia_current_solution.size > 0) || (index < pa_empty.size));

    add_to_int_array(solutions, -2);
    *sol_id = sol_id_local;
    free_wh_bufs(whbufs);
}


int verify_solution(lu_puzzle *p, position_array left, position_array right, position_array top, position_array bottom, position_array center)
{
    unsigned char boolean = 1;
    unsigned int index = 0, lbulb_count = 0;
    position pos;

    unsigned int width = p->width, height = p->height;

    //COINS
    //Coins haut gauche
    if(p->data[0] == lusq_1)
    {
        boolean = boolean && ((p->data[1] == lusq_lbulb) || (p->data[width] == lusq_lbulb));
    }

    //Coins haut droit
    if(p->data[width-1] == lusq_1)
    {
        boolean = boolean && ((p->data[width-2] == lusq_lbulb) || (p->data[2*width-1] == lusq_lbulb));
    }

    //Coins bottom gauche
    if(p->data[width*(height-1)] == lusq_1)
    {
        boolean = boolean && ((p->data[width*(height-2)] == lusq_lbulb) || (p->data[width*(height-1) + 1] == lusq_lbulb));
    }

    //Coins bottom right
    if(p->data[width*height - 1] == lusq_1)
    {
        boolean = boolean && ((p->data[width*(height-1) - 1] == lusq_lbulb) || (p->data[width*height - 2 + 1] == lusq_lbulb));
    }

    //Bord gauche
    for(index = 0; index < left.size && boolean ; index++)
    {
        pos = left.array[index];

        lbulb_count = (
                (p->data[(pos.line-1)*width] == lusq_lbulb) +
                (p->data[(pos.line)*width + 1] == lusq_lbulb) +
                (p->data[(pos.line+1)*width] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }
    
    //Bord droit
    for(index = 0; index < right.size && boolean ; index++)
    {
        pos = right.array[index];

        lbulb_count = (
                (p->data[(pos.line-1)*width] == lusq_lbulb) +
                (p->data[(pos.line)*width - 1] == lusq_lbulb) +
                (p->data[(pos.line+1)*width] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    //Bord haut
    for(index = 0; index < top.size && boolean ; index++)
    {
        pos = top.array[index];

        lbulb_count = (
                (p->data[(pos.line)*width + pos.column - 1] == lusq_lbulb) +
                (p->data[(pos.line)*width + pos.column + 1] == lusq_lbulb) +
                (p->data[(pos.line+1)*width + pos.column] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    //Bord bas
    for(index = 0; index < bottom.size && boolean ; index++)
    {
        pos = bottom.array[index];

        lbulb_count = (
                (p->data[(pos.line)*width + pos.column - 1] == lusq_lbulb) +
                (p->data[(pos.line)*width + pos.column + 1] == lusq_lbulb) +
                (p->data[(pos.line-1)*width + pos.column] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    for(index = 0; index < center.size && boolean ; index++)
    {
        pos = center.array[index];

        lbulb_count = (
                (p->data[(pos.line)*width + pos.column - 1] == lusq_lbulb) +
                (p->data[(pos.line)*width + pos.column + 1] == lusq_lbulb) +
                (p->data[(pos.line-1)*width + pos.column] == lusq_lbulb) +
                (p->data[(pos.line+1)*width + pos.column] == lusq_lbulb));
        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    return boolean;
}

//Essaye la solution (délimité à la fin par un -1). Si à un moment, la solution coince, les ampoules placés
//pour tester la solution sont enlevées.
//Si la solution est possible, la fonction renvoie la taille de la solution
//Si la solution échoue, la fonction renvoie 0
int try_solution(lu_puzzle * p, position_array pa_empty, int * solution)
{
    int i = 0;
    unsigned int width = p->width;
    position pos;

    if(solution[i] == -2) return 0;

    while((solution[i] != -1) && (possible_to_light(p, pa_empty.array[solution[i]]) == 1))
    {
        pos = pa_empty.array[solution[i]];
        p->data[pos.line * width + pos.column] = lusq_lbulb;
        i++;
    }

    if(solution[i] != -1)
    {
        for(i = i-1 ; i >= 0 ; i--)
        {
            pos = pa_empty.array[solution[i]];
            p->data[pos.line * width + pos.column] = lusq_empty;
        }
    }
    return i;
}

//Remove the lightbulb of a solution from the puzzle.
//La fonction revoie la taille de la solution
int remove_solution(lu_puzzle * p, position_array pa_empty, int * solution)
{
    int i = 0;
    unsigned int width = p->width;
    position pos;

    while(solution[i] != -1)
    {
        pos = pa_empty.array[solution[i]];
        p->data[pos.line * width + pos.column] = lusq_empty;
        i++;
    }
    return i++;
}


void remove_impossible(lu_puzzle * p)
{
    unsigned int i = 0, size = p->width * p->height;
    for(i = 0 ; i < size ; i++)
    {
        if(p->data[i] == lusq_impossible) p->data[i] = lusq_empty;
    }
}

void write_solutions(lu_puzzle * p, position_array * classes, int_array * classes_solutions, unsigned int nb_classes, unsigned int *sol_id, FILE *fd)
{
    int i = 0;
    int sol_id_local = 0;
    int * stack = NULL;
    unsigned int stack_size = 0;

    if(*sol_id == 0)
    {
        puzzle_store(p,fd);
        *sol_id = 1;
        return;
    }
    remove_impossible(p);

    stack = (int*) malloc(sizeof(int) * nb_classes);
    
    while((stack_size > 0) || (classes_solutions[stack_size].array[i] != -2))
    {
        while((classes_solutions[stack_size].array[i] != -2) && (try_solution(p, classes[stack_size], &classes_solutions[stack_size].array[i]) == 0))
        {
            i++;
        }
        if(classes_solutions[stack_size].array[i] != -2)
        {
            stack[stack_size] = i;
            stack_size++;
            i = 0;
        }
        else
        {
            --stack_size;
            i = stack[stack_size];
            i += remove_solution(p, classes[stack_size], &classes_solutions[stack_size].array[i]) + 1;

        }
        if(stack_size == nb_classes)
        {
            if(verify_solution(p,left_,right_,top_,bottom_,center_) == 1)
            {
                puzzle_store(p,fd);
                sol_id_local++;
            }

            --stack_size;
            i = stack[stack_size];
            i += remove_solution(p, classes[stack_size], &classes_solutions[stack_size].array[i]) + 1;
        }
    }
    *sol_id = sol_id_local;
}


void solve_classes(lu_puzzle *p, position_array * pa_classes,
        position_array * pa_impossible_classes, unsigned int pa_classes_size,
        unsigned int *sol_id, FILE *fd) 
{
    unsigned int index = 0;
    int_array * classes_solutions;

    classes_solutions = (int_array*)malloc(sizeof(int_array) * pa_classes_size);
    for(index = 0; index < pa_classes_size ; index++)
    {
        solve(p, pa_classes[index] , pa_impossible_classes[index], &classes_solutions[index], sol_id);
    }

    write_solutions(p, pa_classes, classes_solutions, pa_classes_size, sol_id, fd);
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
   if(setvbuf(fd, NULL, _IOFBF, p->width * p->height * 165888) == 0)
   {
       printf("New buffer allocated\n");
   }
   printf("Solving...\n");

   unsigned int nb_e = puzzle_count(p, lusq_empty);
   printf("Problem size = %u\n", nb_e);

   unsigned int sol_id = 0;

   //FIXME
   position_array positions_empty,positions_impossible, left, right, top, bottom, center;
   
   pre_solve(p, &positions_empty, &positions_impossible, &left, &right, &top, &bottom, &center);
   puzzle_print(p);
   left_ = left;
   right_ = right;
   top_ = top;
   bottom_ = bottom;
   center_ = center;

   //int_array solutions;
   //solve(p, positions_empty, positions_impossible, &solutions, &sol_id);

   position_array *pa_array_empty;
   position_array *pa_array_impossible;
   unsigned int pa_array_size;
   classify_positions(p, &pa_array_empty, &pa_array_impossible, &pa_array_size, positions_empty);
   print_class(pa_array_empty,pa_array_size);
   printf("\n");
   print_class(pa_array_impossible,pa_array_size);
   
   printf("Problem size after heuristic = %u\n", positions_impossible.size + positions_empty.size);

   solve_classes(p, pa_array_empty, pa_array_impossible, pa_array_size, &sol_id, fd);
   //FIXME

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

