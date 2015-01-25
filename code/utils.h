#pragma once
#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <sched.h>
#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "lightup.h"
#include "libdvfs.h"

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


void disable_cpu(unsigned int number_cpu_left);
void enable_all_cpu();
void slowdown_cpu(dvfs_ctx * ctx, unsigned int number_cpu_left);
void slowdown(dvfs_ctx * ctx);
