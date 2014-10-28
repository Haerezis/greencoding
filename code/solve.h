#pragma once

#include <stdio.h>
#include "utils.h"
#include "lightup.h"
#include "whbufs.h"

void print_solutions(position_array pa_empty, int * solutions);
void print_solution(position_array pa_empty, int * solutions);
void print_class(position_array* pa_array, unsigned int pa_array_size);

int impossible_to_light(lu_puzzle *p, position pos);
int possible_to_light(lu_puzzle *p, position pos);
/*
 * Regarde si la solution allume tout les emplacements vide
 * (possiblement impossible) que represente pa_empty
 */
int solution_is_complete(lu_puzzle *p, position_array pa_empty, position_array pa_impossible);
int verify_solution(
        lu_puzzle *p,
        position_array left, 
        position_array right,
        position_array top,
        position_array bottom,
        position_array center);
/*
 * Essaye la solution (délimité à la fin par un -1). Si à un moment, la solution coince, les ampoules placés
 * pour tester la solution sont enlevées.
 * Si la solution est possible, la fonction renvoie la taille de la solution
 * Si la solution échoue, la fonction renvoie 0
 */
int try_solution(lu_puzzle * p, position_array pa_empty, int * solution);
/*
 * Remove the lightbulb of a solution from the puzzle.
 * La fonction revoie la taille de la solution.
 */
int remove_solution(lu_puzzle * p, position_array pa_empty, int * solution);
/*
 * Enlève les cases marqué comme impossible du puzzle (pour éviter un conflit avec le checker).
 */
void remove_impossible(lu_puzzle * p);


void classify_positions(
        lu_puzzle *p,
        position_array ** pa_array_empty,
        position_array ** pa_array_impossible,
        unsigned int * pa_array_size,
        position_array pa_empty);
void solve(
        lu_puzzle *p,
        position_array pa_empty,
        position_array pa_impossible,
        int_array * solutions,
        unsigned int *sol_id);
void write_solutions(
        lu_puzzle * p,
        position_array * classes,
        int_array * classes_solutions,
        unsigned int nb_classes, 
        unsigned int *sol_id,
        FILE *fd);
void solve_classes(
        lu_puzzle *p,
        position_array * pa_classes,
        position_array * pa_impossible_classes,
        unsigned int pa_classes_size,
        unsigned int *sol_id,
        FILE *fd);
