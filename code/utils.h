#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "lightup.h"

#define SIZE 1024

typedef struct { unsigned line,column; } position ;

typedef struct { position * array; unsigned int size, max_size ;} position_array;

position_array new_position_array();
position_array new_position_array_with_size(unsigned int size);
void delete_position_array(position_array * pa);
void add_to_position_array(position_array * pa, position p);
inline void remove_from_position_array(position_array * pa, unsigned int index);
unsigned int number_of_lightbulb_possible(lu_puzzle *p, unsigned int line, unsigned int column, position * lb_pos);
char is_alone(lu_puzzle *p, unsigned int line, unsigned int column);

