#pragma once

#include <stdio.h>

/*!
 * lu_square Contenu d'une case du puzzle lightup. Les defines suivants
 * sont utilisés pour les différentes valeurs possibles.
 */
typedef unsigned char lu_square;

/** case bloquée, 0 ampoule autour */
#define   lusq_0 0
/** case bloquée, 1 ampoule autour */
#define   lusq_1 1
/** case bloquée, 2 ampoules autour */
#define   lusq_2 2
/** case bloquée, 3 ampoules autour */
#define   lusq_3 3
/**case bloquée, 4 ampoules autour */
#define   lusq_4 4
/** case bloquée, pas d'indication */
#define   lusq_block_any 5
/** case vide */
#define   lusq_empty 6
/** case avec une ampoule */
#define   lusq_lbulb 7
/** case illuminée */
#define   lusq_enlighted 8
/** case vide mais impossible à remplir */
#define   lusq_impossible 9

/* on aurait pu faire ça avec un 'enum' mais ça aurait stocké un int au
 * lieu d'un simple char... */

/*!
 * \struct lu_puzzle Représente un puzzle lightup complet. Les données du
 * puzzle sont stockées dans le champ data en row major (data[y * width + x]
 * représente la case de coordonnées (x, y). La case (0, 0) est en haut à
 * gauche.
 */
typedef struct {
   unsigned int width;     /*!< largeur (en nombre de cases) */
   unsigned int height;    /*!< hauteur (en nombre de cases) */
   lu_square *data;        /*!< contenu de la grille */
} lu_puzzle;

/*!
 * Charge un puzzle depuis un fichier dont le nom est passé en argument.
 *
 * \param path Le chemin vers le fichier à charger.
 * \param no numéro du puzzle à charger s'il y en a plusieurs dans le fichier (solution). premier = 1.
 * \return Un puzzle représentant les données dans le fichier ou NULL en cas 
 * d'erreur.
 */
lu_puzzle *puzzle_load(const char *path, const unsigned int no);

/*!
 * ouvre un fichier contenant les solutions à un puzzle
 *
 * \param path nom du fichier
 * \param width pointeur vers la largeur du puzzle
 * \param height pointeur vers la hauteur du puzzle
 * \return le fichier ouvert
 */
FILE *puzzle_open_sol_file(const char *path, unsigned int *width, unsigned int *height);

/*!
 * Charge un puzzle depuis un fichier ouvert par puzzle_open_sol_file()
 *
 * \param fd le fichier
 * \param width largeur du puzzle
 * \param height hauteur du puzzle
 * \return Un puzzle représentant les données du puzzle suivant stocké dans le fichier
 * ou NULL en cas d'erreur/fin de fichier.
 */
lu_puzzle *puzzle_load_next_sol(FILE *fd, const unsigned int width, const unsigned int height);

/*!
 * Ouvre un fichier en écriture pour y stocker les solutions.
 *
 * \param fname nom du fichier
 * \param width Largeur du puzzle.
 * \param height Hauteur du puzzle.
 */
FILE *puzzle_open_storage_file(char *fname, unsigned int width, unsigned int height);

/*!
 * Écrit un puzzle dans un fichier.
 *
 * \param p Le puzzle à stocker.
 * \param fd fichier destination.
 */
void puzzle_store(const lu_puzzle *p, FILE *fd);

/*!
 * Libère la mémoire utilisée par un puzzle.
 *
 * \param p Le puzzle à libérer.
 */
void puzzle_destroy(lu_puzzle *p);

/*!
 * Affiche un puzzle à l'écran. L'affichage est prévu pour être lisible par un
 * humain. Cette fonction n'est pas thread-safe.
 *
 * \param p Le puzzle à afficher.
 */
void puzzle_print(const lu_puzzle *p);

/*!
 * Génère un puzzle vide aux dimensions spécifiées.
 *
 * \param width Largeur du puzzle.
 * \param height Hauteur du puzzle.
 * \return Un puzzle aux dimensions spécifiées, remplit de cases vides.
 */
lu_puzzle *puzzle_new(unsigned int width, unsigned int height);

/*!
 * Duplique un puzzle.
 *
 * \param p Le puzzle à copier.
 * \return Une copie de \p p.
 */
lu_puzzle *puzzle_clone(const lu_puzzle *p);

/*!
 * Remplace les cases vides par des cases "enlighted" quand elles sont
 * éclairées par l'ampoule dont les coordonnées sont spécifiées.
 *
 * \param p Le puzzle à modifier.
 * \param x Abscisse de l'ampoule à "allumer".
 * \param y Ordonnée de l'ampoule à "allumer".
 */
__inline void puzzle_light_on(lu_puzzle *p, unsigned int x, unsigned int y);

/*!
 * Remplace les cases vides par des cases "enlighted" quand elles sont
 * éclairées par une ampoule quelconque.
 *
 * \param p Le puzzle à modifier.
 */
void puzzle_lights_on(lu_puzzle *p);

/*!
 * Remplace les cases "englighted" par des cases "empty".
 *
 * \param p Le puzzle à modifier.
 */
void puzzle_lights_off(lu_puzzle *p);

/*!
 * Compte le nombre de cases du type spécifié.
 *
 * \param p Le puzzle dans lequel compter les cases.
 * \param tp Le type des cases à compter.
 * \return Le nombre de cases du type \p tp dans \p p.
 */
unsigned int puzzle_count(const lu_puzzle *p, lu_square tp);

/*!
 * Compte le nombre de mur 1/2/3 et de case vide
 */
void puzzle_count_1_2_3_wall_and_empty(const lu_puzzle *p,
        unsigned int * wall_number, unsigned int * empty_number);

/*!
 * Vérifie que la solution d'un puzzle est valide. Teste si la structure du
 * puzzle n'a pas changé et vérifie les contraintes du lightup (toutes les
 * cases allumées, pas d'ampoules face à face et respect des contraintes 
 * d'adjacence).
 *
 * \param ref Le problème de référence dont \p sol est la solution.
 * \param sol Solution au problème \p ref.
 *
 * \return 1 si la solution est correcte, 0 sinon.
 */
unsigned int puzzle_check(const lu_puzzle *ref, lu_puzzle *sol);

/*!
 * Renvoie un hash pour le puzzle. Le hash sera identique que les ampoules
 * soient allumées ou non.
 *
 * \param p Le puzzle à hasher.
 * \return Un hash de \p p.
 */
unsigned int puzzle_hash(const lu_puzzle *p);

/*!
 * Test si deux puzzle sont identiques. La fonction ne tient pas compte des
 * cases éclairées et les considère comme vides.
 *
 * \param ref Le premier puzzle à tester.
 * \param cmp Le deuxième puzzle à test.
 *
 * \return 1 si les deux puzzle sont identiques, 0 sinon.
 */
unsigned int puzzle_eq(const lu_puzzle *ref, const lu_puzzle *cmp);
