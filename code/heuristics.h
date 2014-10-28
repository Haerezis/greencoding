#pragma once

#include <stdio.h>
#include "lightup.h"
#include "utils.h"
#include "whbufs.h"

/*
 * Essaye de remplir les cases autours des murs selons leur contraintes.
 * Les contraintes doivent être sûr (3 case de libre pour un mur 3 par exemple).
 */
char wall_heuristic(lu_puzzle *p,
        position_array * pa_left_border,
        position_array * pa_right_border,
        position_array * pa_top_border,
        position_array * pa_bottom_border,
        position_array * pa_center);

/*
 * Essaye de remplir les cases vides et impossible selons leur contraintes.
 * Par exemple, si une case impossible (rendu impossible par un mur) n'est éclairable
 * que depuis une case vide en particulier, cette fonction allumera cette case.
 * On eclaire aussi les cases vides qui ne peuvent pas être éclairé autre que par
 * elle même.
 */
char empty_and_impossible_heuristic(
        lu_puzzle * p,
        position_array * pa_empty,
        position_array * pa_impossible);

/*
 * Pre-solve le puzzle en appelant en boucle les différentes fonctions heuristiques,
 * jusqu'à ce qu'il n'y ai plus de changement faites par ces dernières.
 */
void pre_solve(lu_puzzle *p,
        position_array * positions_empty,
        position_array * positions_impossible,
        position_array * left,
        position_array * right,
        position_array * top,
        position_array * bottom,
        position_array * center);
