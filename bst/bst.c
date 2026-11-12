//  bst.c : partie implantation d'un module polymorphe pour la spécification
//   ABINR du TDA ABinR(T).

#include "bst.h"

//=== Type cbst ================================================================

//--- Définition cbst ----------------------------------------------------------

typedef struct cbst cbst;

struct cbst {
  cbst *next[2];
  const void *ref;
  int height;
  size_t size;
};

//--- Raccourcis cbst ----------------------------------------------------------

#define EMPTY()     nullptr
#define IS_EMPTY(p) ((p) == nullptr)
#define LEFT(p)     ((p)->next[0])
#define RIGHT(p)    ((p)->next[1])
#define REF(p)      ((p)->ref)
#define HEIGHT(p) ((p)->height)
#define NEXT(p, d)  ((p)->next[(d) > 0])
#define SIZE(p)     ((p)->size)

//--- Divers -------------------------------------------------------------------

static int max__int(int x1, int x2) {
  return x1 > x2 ? x1 : x2;
}

static size_t min__size_t(size_t x1, size_t x2) {
  return x1 < x2 ? x1 : x2;
}

//--- Fonctions cbst -----------------------------------------------------------

//  DEFUN_CBST__MEASURE : définit la fonction récursive de nom « cbst__ ## fun »
//    et de paramètre un pointeur d'arbre binaire, qui renvoie zéro si l'arbre
//    est vide et « 1 + oper(r0, r1) » sinon, où r0 et r1 sont les valeurs
//    renvoyées par les appels récursifs de la fonction avec les pointeurs des
//    sous-arbres gauche et droit de l'arbre comme paramètres.
#define DEFUN_CBST__MEASURE(fun, oper)                                         \
  static size_t cbst__ ## fun(const cbst * p) {                                \
    return IS_EMPTY(p)                                                         \
      ? 0                                                                      \
      : 1 + oper(cbst__ ## fun(LEFT(p)), cbst__ ## fun(RIGHT(p)));             \
  }

//  cbst__distance : définitions.

DEFUN_CBST__MEASURE(distance, min__size_t)

//  cbst__size : renvoie la taille de l'arbre binaire de recherche repéré par p
static size_t cbst__size(const cbst *p) {
  return IS_EMPTY(p) ? 0 : SIZE(p);
}

//  cbst__update_size : met à jour la taille de l'arbre binaire
//    de recherche pointé par p. Il est supposé que p n'est pas un pointeur
//    vers un arbre vide.
static void cbst__update_size(cbst *p) {
  SIZE(p) = cbst__size(LEFT(p)) + cbst__size(RIGHT(p)) + 1;
}

//  cbst__height : renvoie la hauteur de l'arbre binaire de recherche repéré
//    par p.
static int cbst__height(const cbst *p) {
  return IS_EMPTY(p) ? 0 : HEIGHT(p);
}

//  cbst__update_height : met à jour la hauteur de l'arbre binaire
//    de recherche pointé par p. Il est supposé que p n'est pas un pointeur
//    vers un arbre vide.
static void cbst__update_height(cbst *p) {
  HEIGHT(p) = 1 + max__int(cbst__height(LEFT(p)), cbst__height(RIGHT(p)));
}

//  cbst__balance : renvoie l'équilibre de l'arbre binaire de recherche
//    repéré par p.
static int cbst__balance(const cbst *p) {
  return IS_EMPTY(p) ? 0 : cbst__height(LEFT(p)) - cbst__height(RIGHT(p));
}

//  cbst__rotation_left : suppose que l'arbre binaire de recherche repéré par
//    *pp supporte la rotation gauche et l'effectue.
static void cbst__rotation_left(cbst **pp) {
  cbst *t = *pp;
  *pp = RIGHT(*pp);
  RIGHT(t) = LEFT(*pp);
  LEFT(*pp) = t;
  cbst__update_height(LEFT(*pp));
  cbst__update_height(*pp);
  cbst__update_size(LEFT(*pp));
  cbst__update_size(*pp);
}

//  cbst__rotation_left : suppose que l'arbre binaire de recherche repéré par
//    *pp supporte la rotation droite etl'effectue.
static void cbst__rotation_right(cbst **pp) {
  cbst *t = *pp;
  *pp = LEFT(*pp);
  LEFT(t) = RIGHT(*pp);
  RIGHT(*pp) = t;
  cbst__update_height(RIGHT(*pp));
  cbst__update_height(*pp);
  cbst__update_size(RIGHT(*pp));
  cbst__update_size(*pp);
}

//  cbst__rotation_left_right : suppose que l'arbre binaire de recherche repéré
//     par *pp supporte la rotation composée gauche droite et l'effectue.
static void cbst__rotation_left_right(cbst **pp) {
  cbst__rotation_left(&LEFT(*pp));
  cbst__rotation_right(pp);
}

//  cbst__rotation_left_right : suppose que l'arbre binaire de recherche repéré
//     par *pp supporte la rotation composée droite gauche et l'effectue.
static void cbst__rotation_right_left(cbst **pp) {
  cbst__rotation_right(&RIGHT(*pp));
  cbst__rotation_left(pp);
}

// cbst__balancing : renvoie une valeur nulle si l'arbre binaire de recherche
//    repéré par *pp est équilibré, dans le cas contraire effectue la rotation
//    appropriée puis renvoie une valeur non nulle
static int cbst__balancing(cbst **pp) {
  cbst__update_height(*pp);
  int c = cbst__balance(*pp);
  if (-1 <= c && c <= 1) {
    return 0;
  }
  if (c == 2) {
    if (cbst__balance(LEFT(*pp)) >= 0) {
      cbst__rotation_right(pp);
    } else {
      cbst__rotation_left_right(pp);
    }
  } else {
    if (cbst__balance(RIGHT(*pp)) <= 0) {
      cbst__rotation_left(pp);
    } else {
      cbst__rotation_right_left(pp);
    }
  }
  return 1;
}

//  cbst__dispose : libère les ressources allouées à l'arbre binaire pointé par
//    p.
static void cbst__dispose(cbst *p) {
  if (!IS_EMPTY(p)) {
    cbst__dispose(LEFT(p));
    cbst__dispose(RIGHT(p));
    free(p);
  }
}

//  cbst__remove_max : supprime la valeur maximale dans l'arbre binaire
//    de recherche pointé par *pp et renvoie cette valeur. Il est supposé que
//    l'arbre n'est pas vide.

static void *cbst__remove_max(cbst **pp) {
  if (IS_EMPTY(RIGHT(*pp))) {
    const void *r = REF(*pp);
    cbst *t = *pp;
    *pp = LEFT(*pp);
    free(t);
    return (void *) r;
  }
  void *r = cbst__remove_max(&RIGHT(*pp));
  cbst__update_size(*pp);
  cbst__balancing(pp);
  return r;
}

//  cbst__remove_root : supprime la racine de l'arbre binaire de recherche
//    pointée par *pp. Il est supposé que l'arbre n'est pas vide.
static void cbst__remove_root(cbst **pp) {
  if (IS_EMPTY(LEFT(*pp))) {
    cbst *t = *pp;
    *pp = RIGHT(*pp);
    free(t);
    return;
  }
  REF(*pp) = cbst__remove_max(&LEFT(*pp));
  cbst__update_size(*pp);
  cbst__balancing(pp);
}

//  cbst__remove_climbup_left : supprime de l'arbre binaire de recherche associé
//    à *pp la référence qui est égale à la ref au sens de compar par la méthode
//    de retrait par remontée gauche. Il est supposé que ref est différent d'un
//    pointeur nul.
static void *cbst__remove_climbup_left(cbst **pp, const void *ref,
    int (*compar)(const void *, const void *)) {
  if (IS_EMPTY(*pp)) {
    return nullptr;
  }
  int c = compar(ref, REF(*pp));
  if (c == 0) {
    const void *r = REF(*pp);
    cbst__remove_root(pp);
    return (void *) r;
  }
  void *r = cbst__remove_climbup_left(&NEXT(*pp, c), ref, compar);
  cbst__update_size(*pp);
  cbst__balancing(pp);
  return r;
}

//  cbst__add_endofpath : recherche  dans l'arbre binaire de recherche pointée
//    par pp la référence d'un objet égal à celui de référence ref au sens de la
//    fonction de  comparaison pointée par compar. Si la recherche est positive,
//    renvoie la référence trouvée. Tente sinon d'ajouter la référence selon la
//    méthode de l'ajout en bout de chemin ; renvoie un pointeur nul en cas de
//    dépassement de capacité ; renvoie sinon ref.
static void *cbst__add_endofpath(cbst **pp, const void *ref,
    int (*compar)(const void *, const void *)) {
  if (IS_EMPTY(*pp)) {
    *pp = malloc(sizeof **pp);
    if (*pp == nullptr) {
      return nullptr;
    }
    LEFT(*pp) = EMPTY();
    RIGHT(*pp) = EMPTY();
    REF(*pp) = ref;
    cbst__update_size(*pp);
    cbst__balancing(pp);
    return (void *) ref;
  }
  int c = compar(ref, REF(*pp));
  if (c == 0) {
    return (void *) (REF(*pp));
  }
  void *r = cbst__add_endofpath(&NEXT(*pp, c), ref, compar);
  cbst__update_size(*pp);
  cbst__balancing(pp);
  return r;
}

//  cbst__search : recherche dans l'arbre binaire ordonnée repéré par p
//    une référence égale à ref au sens de la fonction de comparaison pointé
//    par compar. En cas de recherche négative renvoie un pointeur nul.
//    En cas de recherche positive renvoie la référence trouvé.
static void *cbst__search(const cbst *p, const void *ref,
    int (*compar)(const void *, const void *)) {
  if (IS_EMPTY(p)) {
    return nullptr;
  }
  int c = compar(ref, REF(p));
  if (c == 0) {
    return (void *) REF(p);
  }
  return cbst__search(NEXT(p, c), ref, compar);
}

//  cbst__number : recherche dans l'arbre binaire de recherche repéré par p
//    une référence égale à ref au sens de la fonction de comparaison pointé par
//    compar. En cas de recherche positive, renvoie le numéro du noeud
//    contenant cette référence. En cas de recherche négative renvoie le numéro
//    qu'aurait le noeud si la référence était ajoutée à l'arbre.
//    Il est supposé que ref n'est pas un pointeur nul.
static size_t cbst__number(const cbst *p, const void *ref,
    int (*compar)(const void *, const void *), size_t number) {
  if (IS_EMPTY(p)) {
    return number;
  }
  int c = compar(ref, REF(p));
  if (c == 0) {
    return number;
  }
  return cbst__number(NEXT(p, c), ref, compar, 2 * number + (c > 0));
}

//  cbst__rank : recherche dans l'arbre binaire de recherche répéré par p une
//    référence égale à ref au sens de la fonction de comparaison pointé par
//    compar. En cas de recherche positive, renvoie le rang du noeud
//    contenant cette référence. En cas de recherche négative renvoie le rang
//    du noeud qu'aurait le noeud si la référence était ajoutée à l'arbre.
//    Il est supposé que ref n'est pas un pointeur nul.

//    rang-valeur : ABinR(T ) × T ̸→ Naturel

static size_t cbst__rank(const cbst *p, const void *ref, int (*compar)(
    const void *, const void *), size_t rank) {
  if (IS_EMPTY(p)) {
    return rank;
  }
  int c = compar(ref, REF(p));
  if (c == 0) {
    return rank + cbst__size(LEFT(p));
  }
  return cbst__rank(NEXT(p, c), ref, compar, rank + cbst__size(LEFT(
      p)) + (c > 0));
}

#define REPR__TAB 4

//  cbst__repr_graphic : affiche la représentation graphique par rotation
//    antihoraire d'un quart de tour du sous-arbre binaire de recherche p avec
//    une indentation par niveau de REPR_TAB caractères. Le niveau du sous-arbre
//    est supposé être la valeur de level. La fonction d'affichage des
//    références est pointé par put.
static void cbst__repr_graphic(const cbst *p,
    void (*put)(const void *ref), int level) {
  if (IS_EMPTY(p)) {
    printf("%*s|\n", REPR__TAB * (int) level, "");
  } else {
    cbst__repr_graphic(RIGHT(p), put, level + 1);
    printf("%*s", REPR__TAB * (int) level, "");
    put(p->ref);
    printf(" h=%d b=%d \n", cbst__height(p), cbst__balance(p));
    cbst__repr_graphic(LEFT(p), put, level + 1);
  }
}

int cbst_dft_infix_apply_context(cbst *p, int dir,
    void *context, int (*fun)(void *context, const void *ref),
    int (*fun_pre)(void *context), int (*fun_post)(void *context)) {
  if (!IS_EMPTY(p)) {
    if (dir <= 0) {
      cbst_dft_infix_apply_context(LEFT(p), dir, context, fun, fun_pre,
          fun_post);
      if (fun_pre != nullptr) {
        int r = fun_pre(context);
        if (r != 0) {
          return r;
        }
      }
      int r = fun(context, REF(p));
      if (r != 0) {
        return r;
      }
      cbst_dft_infix_apply_context(RIGHT(
          p), dir, context, fun, fun_pre, fun_post);
      if (fun_post != nullptr) {
        int r = fun_post(context);
        if (r != 0) {
          return r;
        }
      }
    } else {
      cbst_dft_infix_apply_context(RIGHT(
          p), dir, context, fun, fun_pre, fun_post);
      if (fun_pre != nullptr) {
        int r = fun_pre(context);
        if (r != 0) {
          return r;
        }
      }
      int r = fun(context, REF(p));
      if (r != 0) {
        return r;
      }
      cbst_dft_infix_apply_context(LEFT(p), dir, context, fun, fun_pre,
          fun_post);
      if (fun_post != nullptr) {
        int r = fun_post(context);
        if (r != 0) {
          return r;
        }
      }
    }
  }
  return 0;
}

//=== Type bst =================================================================

//--- Définition bst -----------------------------------------------------------

struct bst {
  int (*compar)(const void *, const void *);
  cbst *root;
};

//--- Fonctions bst ------------------------------------------------------------

bst *bst_empty(int (*compar)(const void *, const void *)) {
  bst *t = malloc(sizeof *t);
  if (t == nullptr) {
    return nullptr;
  }
  t->compar = compar;
  t->root = EMPTY();
  return t;
}

void bst_dispose(bst **tptr) {
  if (*tptr == nullptr) {
    return;
  }
  cbst__dispose((*tptr)->root);
  free(*tptr);
  *tptr = nullptr;
}

void *bst_remove_climbup_left(bst *t, const void *ref) {
  if (ref == nullptr) {
    return nullptr;
  }
  void *x = bst_search(t, ref);
  if (x == nullptr) {
    return nullptr;
  }
  return cbst__remove_climbup_left(&t->root, ref, t->compar);
}

void *bst_add_endofpath(bst *t, const void *ref) {
  if (ref == nullptr) {
    return nullptr;
  }
  return cbst__add_endofpath(&t->root, ref, t->compar);
}

void *bst_search(bst *t, const void *ref) {
  if (ref == nullptr) {
    return nullptr;
  }
  return cbst__search(t->root, ref, t->compar);
}

size_t bst_size(bst *t) {
  return cbst__size(t->root);
}

size_t bst_height(bst *t) {
  return (size_t) cbst__height(t->root);
}

size_t bst_distance(bst *t) {
  return cbst__distance(t->root);
}

size_t bst_number(bst *t, const void *ref) {
  if (ref == nullptr) {
    return (size_t) -1;
  }
  return cbst__number(t->root, ref, t->compar, 1);
}

size_t bst_rank(bst *t, const void *ref) {
  if (ref == nullptr) {
    return (size_t) -1;
  }
  if (IS_EMPTY(t->root)) {
    return 0;
  }
  return cbst__rank(t->root, ref, t->compar, cbst__size(LEFT(t->root)));
}

void bst_repr_graphic(bst *t, void (*put)(const void *ref)) {
  cbst__repr_graphic(t->root, put, 1);
}

//--- Raccourcis
// infix_apply_context-------------------------------------------------------

#define EMPTY_CONTROLLER 0

int bst_dft_infix_apply_context(bst *t, int dir,
    void *context, int (*fun)(void *context, const void *ref),
    int (*fun_pre)(void *context), int (*fun_post)(void *context)) {
  if (t == nullptr) {
    return EMPTY_CONTROLLER;
  }
  return cbst_dft_infix_apply_context(t->root, dir, context, fun, fun_pre,
      fun_post);
}
