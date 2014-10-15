#pragma once

// liste chainée de puzzles
struct puzzle_list_s;

typedef struct puzzle_list_s {
   lu_puzzle *puzzle;
   struct puzzle_list_s *next;
} puzzle_list;

/** 
 * Fonction principale du checker.
 * Appelée depuis le main sous Linux ou via ISDA sous windows.
 */
int checker_main(int argc, char **argv);
