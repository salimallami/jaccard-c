// jdis.h : partie interface d'un module permettant de créer ou supprimer
// une cellule ainsi que de calculer la distance de jacard

#ifndef WORD_CELL__H
#define WORD_CELL__H

#include <stdio.h>
#include <stdlib.h>
#include "../bst/bst.h"

//  Fonctionnement général :
//  - la structure de données stocke un objets le tableau de boolean
// represantant l'appartenance ou non des mot au fichier et une références vers
//      le mot. Les références sont du type générique « const void * » ;
//  - si des opérations d'allocation dynamique sont effectuées, elles le sont
//      pour la gestion propre de la structure de données, et en aucun cas pour
//      réaliser des copies ou des destructions d'objets ;
//  - les fonctions qui possèdent un paramètre de type « wordd_cell * » ou «
// word_cell ** »
//      ont un comportement indéterminé lorsque ce paramètre ou sa déréférence
//      n'est pas l'adresse d'un contrôleur préalablement renvoyée avec succès
//      par la fonction word_cell_empty et non révoquée depuis par la fonction
//      word_cell_dispo ;
//  - aucune fonction ne peut ajouter un pointeur nul à la structure de
//      données ;
//  - l'implantation des fonctions dont la spécification ne précise pas qu'elles
//      doivent gérer les cas de dépassement de capacité ne doivent avoir
//      affaire à aucun problème de la sorte.

//  struct word_cell, word_cell : type et nom de type d'un contrôleur regroupant
// les informations nécessaires pour gérer un mot et son appartenance
//à x fichier
typedef struct word_cell word_cell;

//  word_cell_empty :  tente d'allouer les ressources nécessaires pour gérer
//  un nouveau word_cell initialement vide. Initialise le valeur de ref par la
//  reference word et initialise un vecteur de booléens de taille n à false.
//    Renvoie un pointeur nul en cas de dépassement de capacité. Renvoie sinon
//    un pointeur vers le contrôleur associé à l'arbre.
extern word_cell *word_cell_empty(const void *word, size_t n);

//  word_cell_dispo : libère les ressources associées à un pointeur wc_ptr
//  Libère sinon les ressources allouées à la gestion de la cellule
//     associé à *wc_ptr.
//    Renvoie 0 toujours.
extern int word_cell_dispo(void *wc_ptr);

//  app_to_file : marque la présence du mot dans un fichier donné.
//    Ne retourne rien et ne fait pas d’allocation supplémentaire.
extern void app_to_file(word_cell *wc, size_t number);

//  word_cell_compar : compare deux cellules sur leur chaîne de caractères.
//    Utilise strcoll((char*)a->ref, (char*)b->ref) pour respecter
//    l’ordre lexicographique courant selon la locale.
//    Renvoie <0 si a < b, 0 si égal, >0 si a > b.
extern int word_cell_compar(const word_cell *a, const word_cell *b);

//  bst_jaccard : calcule la dissimilarité de Jaccard entre deux fichiers
//    indexés dans l’arbre. Renvoie 1.0 – intersection/uni.
//    Ne modifie pas l’arbre et n’alloue pas de mémoire.
extern double bst_jaccard(bst *tree, int f1, int f2);

//  jdis_graph : affichage graphique d’une ligne de table parmi les n fichier
// inscrit dans ligne de commande.
//    Imprime sur stdout le mot suivi d’une tabulation, puis pour chaque
//    index i de 0 à *n_files-1 imprime “x” si wc->app[i] est true,
//    sinon “-”, chacun suivi d’une tabulation, puis un retour à la ligne.
//    Renvoie 0 pour indiquer la poursuite du parcours.
extern int jdis_graph(size_t *n_files, word_cell *wc);

#endif
