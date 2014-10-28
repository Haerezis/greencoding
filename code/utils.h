#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "lightup.h"

#define SIZE 1024

typedef struct { unsigned line,column; } position ;

typedef struct { position * array; unsigned int size, max_size ;} position_array;

typedef struct { int * array; unsigned int size,max_size;} int_array;

position_array new_position_array();
position_array new_position_array_with_size(unsigned int size);
void delete_position_array(position_array * pa);
void add_to_position_array(position_array * pa, position p);
inline void remove_from_position_array(position_array * pa, unsigned int index);
unsigned int number_of_lightbulb_possible(lu_puzzle *p, unsigned int line, unsigned int column, position * lb_pos);
char is_alone(lu_puzzle *p, unsigned int line, unsigned int column);

int_array new_int_array();
int_array new_int_array_with_size(unsigned int size);
void delete_int_array(int_array * ia);
void add_to_int_array(int_array * ia, int p);


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
   unsigned int y);
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
   unsigned int y);


void disable_cpu(unsigned int number_cpu_left);
void enable_all_cpu();
