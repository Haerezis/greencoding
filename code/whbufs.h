#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
wh_bufs *new_wh_bufs(unsigned int width, unsigned int height, unsigned int size);

/*!
 * Affecte une ligne et une colonne depuis le pool de buffers.
 *
 * \param bufs Le pool de lignes et colonnes.
 * \param[out] wbuf Ligne allouée dans le pool.
 * \param[out] hbuf Colonne allouée dans le pool.
 */
void pop_wh_buf(wh_bufs *bufs, char **wbuf, char **hbuf);

/*!
 * Reaffecte la ligne et la colonne affecté au dernier pop
 *
 * \param bufs Le pool de lignes et colonnes.
 * \param[out] wbuf Ligne affectée la dernière fois.
 * \param[out] hbuf Colonne affectée la dernière fois.
 */
void get_head_wh_buf(wh_bufs *bufs, char **wbuf, char **hbuf);


/*!
 * Libère la dernière ligne et la dernière colonne affectée dans le pool. Cette
 * ligne et cette colonne sont désormais réutilisables.
 *
 * \param bufs Le pool qui possède la dernière ligne et colonne affectées.
 */
void release_wh_buf(wh_bufs *bufs);

/*!
 * Libére la mémoire allouée par un pool de lignes et colonnes.
 *
 * \param bufs Le pool de buffers à libérer.
 */
void free_wh_bufs(wh_bufs *bufs);

