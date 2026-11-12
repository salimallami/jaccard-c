// jdis.h : partie implémentation d'un module permettant de créer ou supprimer
// une cellule de texte ainsi que de calculer la distance de jacard

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../bst/bst.h"

#define GRAPH_APP "x"
#define GRAPH_NO_APP "-"

//=== Type word_cell ===========================================================

//--- Définition word_cell -----------------------------------------------------

typedef struct word_cell word_cell;

struct word_cell {
  const void *ref;
  bool app[];
};

//--- Fonctions word_cell ------------------------------------------------------

word_cell *word_cell_empty(const void *word, size_t n) {
  word_cell *wc = malloc(sizeof *wc + sizeof(bool) * n);
  if (wc == nullptr) {
    return nullptr;
  }
  wc->ref = word;
  for (size_t k = 0; k < n; k++) {
    wc->app[k] = false;
  }
  return wc;
}

int word_cell_dispo(void *wc) {
  word_cell *wcb = (word_cell *) wc;
  free((void *) wcb->ref);
  free(wcb);
  return 0;
}

void app_to_file(word_cell *wc, size_t number) {
  wc->app[number] = true;
}

int word_cell_compar(const word_cell *a, const word_cell *b) {
  return strcoll(a->ref, b->ref);
}

//=== Type jdis ================================================================

//--- Définition jdis ----------------------------------------------------------

//  struct jdis, jdis : type et nom de type d'un contrôleur regroupant
// les informations nécessaires pour gérer le calcul de la distance de jaccard
// avec une structure qui contient un couple d'entier representant sur quelle
// fichier la distance est calculer et deux size_t representant l'intersection
// des deux ensembles(resp.l'union).
typedef struct jdis jdis;

struct jdis {
  int f1, f2;
  size_t intersection;
  size_t uni;
};

//--- Fonctions jdis -----------------------------------------------------------

// jaccard : incremente intersection si le mot est present dans les deux fichier
// et increment uni si au moins l'un des fichier contient le mot
static int jaccard(void *context, const void *ref) {
  jdis *ctx = context;
  const word_cell *wc = ref;
  int in1 = wc->app[ctx->f1];
  int in2 = wc->app[ctx->f2];
  if (in1 || in2) {
    ctx->uni++;
    if (in1 && in2) {
      ctx->intersection++;
    }
  }
  return 0;
}

double bst_jaccard(bst *tree, int f1, int f2) {
  jdis ctx = {
    .f1 = f1, .f2 = f2,
    .intersection = 0,
    .uni = 0
  };
  bst_dft_infix_apply_context(tree,
      0,
      &ctx,
      jaccard,
      NULL,
      NULL);
  if (ctx.uni == 0) {
    return 1.0;
  }
  return 1.0 - (double) ctx.intersection / (double) ctx.uni;
}

int jdis_graph(size_t *n, word_cell *word) {
  printf("%s\t", (char *) word->ref);
  for (size_t i = 0; i < *n; ++i) {
    if (word->app[i]) {
      printf("%s\t", GRAPH_APP);
    } else {
      printf("%s\t", GRAPH_NO_APP);
    }
  }
  printf("\n");
  return 0;
}
