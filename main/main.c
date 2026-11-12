//main : point d’entrée de jdis.
//  - Analyse les options de ligne de commande
//  (–help, –usage, –version, –i, –g).
//  - Lit un ou plusieurs fichiers, extrait les mots et construit un BST
//    indexant leur présence dans chaque fichier.
//  - En mode normal, calcule et affiche les dissimilarités de Jaccard pour
//    chaque paire de fichiers.
//  - En mode graphique (-g), affiche un tableau binaire des apparitions de
//    chaque mot dans les fichiers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bst.h"
#include "jdis.h"
#include "holdall.h"

//──────────────────────────────────────────────────────────────────────────────
// OPTIONS NAMES
//──────────────────────────────────────────────────────────────────────────────
#define HELP_OPT             "--help"
#define HELP_OPT_BIS         "-?"
#define USAGE_OPT            "--usage"
#define VERSION_OPT          "-version"
#define INITIAL_OPT          "--initial="
#define INITIAL_OPT_BIS      "-i"
#define PONCTUATION_OPT      "-punctuation-like-space"
#define PONCTUATION_OPT_BIS  "-p"
#define GRAPH_OPT            "--graph"
#define GRAPH_OPT_BIS        "-g"

//──────────────────────────────────────────────────────────────────────────────
// OPTIONS TEXTS
//──────────────────────────────────────────────────────────────────────────────
#define USAGE_TEXT   "Usage: jdis [OPTION]... FILE1 FILE2 [FILE]...\n"
#define VERSION_TEXT                                                           \
  "jdis - 25.05.7\n"                                                           \
  "This is freeware: you can redistribute it. There is NO WARRANTY.\n"         \
  "Salim ALLAMI\n"

//──────────────────────────────────────────────────────────────────────────────
// DEFINE
//──────────────────────────────────────────────────────────────────────────────
#define MIN_CAPACITY 16
#define MUL_CAPACITY 2

//──────────────────────────────────────────────────────────────────────────────
// FONCTIONS
//──────────────────────────────────────────────────────────────────────────────

//  compar_option : détermine si une chaîne de caractères représente une option
//    de ligne de commande.
//    - s : pointeur vers une chaîne non nulle.
//    Renvoie true si le premier caractère de s est ‘-’ et que la chaîne fait
//    au moins deux caractères de long (pour éviter de traiter “-” seul comme
//    option valide). Renvoie false sinon.
bool compar_option(char *s);

//  help : affiche sur la sortie standard un message d’aide complet décrivant
//    l’utilisation du programme.
void help();

//==============================================================================
// FONCTIONS
//==============================================================================
int main(int argc, char *argv[]) {
  int r = EXIT_SUCCESS;
  char **file_names = nullptr;
  size_t file_number = 0;
  bst *tree = nullptr;
  holdall *ht = nullptr;
  //-- OPT MANAGEMENT ----------------------------------------------------------
  bool graph = false;
  size_t maxlen = 0;
  for (int i = 1; i < argc; i++) {
    if (!compar_option(argv[i])) {
      continue;
    }
    if (strcmp(argv[i], HELP_OPT) == 0
        || strcmp(argv[i], HELP_OPT_BIS) == 0) {
      help();
      goto dispose;
    } else if (strcmp(argv[i], USAGE_OPT) == 0) {
      fprintf(stdout, "%s", USAGE_TEXT);
      goto dispose;
    } else if (strcmp(argv[i], VERSION_OPT) == 0) {
      printf("%s", VERSION_TEXT);
      goto dispose;
    } else if (strcmp(argv[i], GRAPH_OPT) == 0
        || strcmp(argv[i], GRAPH_OPT_BIS) == 0) {
      graph = true;
    } else if (strncmp(argv[i], INITIAL_OPT_BIS, 2) == 0
        || strncmp(argv[i], INITIAL_OPT, 10) == 0) {
      char *val = NULL;
      if (strncmp(argv[i], INITIAL_OPT, 10) == 0) {
        if (argv[i][10] == '\0') {
          fprintf(stderr, "%s: option '%s' requires a numeric argument\n",
              argv[0], INITIAL_OPT);
          goto error;
        }
        val = argv[i] + 10;
      } else if (strncmp(argv[i], INITIAL_OPT_BIS, 2) == 0) {
        if (argv[i][2] == '\0') {
          fprintf(stderr, "%s: option '%s' requires a numeric argument\n",
              argv[0], INITIAL_OPT_BIS);
          goto dispose;
        }
        val = argv[i] + 2;
      }
      char *endptr;
      long v = strtol(val, &endptr, 10);
      if (endptr == val) {
        fprintf(stderr, "Invalid number: %s\n", val);
        goto error;
      }
      if (*endptr != '\0' || v < 0) {
        fprintf(stderr, "%s: invalid value for '-i': %s\n", argv[0], val);
        goto error;
      }
      if (v == 0) {
        maxlen = (size_t) v;
      }
    } else {
      fprintf(stderr, "%s: Unrecognized option '%s'\n", argv[0], argv[i]);
      fprintf(stderr, "Try '%s " HELP_OPT_BIS "' for more information\n",
          argv[0]);
      goto error;
    }
  }
  //-- FILE INIT ---------------------------------------------------------------
  file_names = malloc((size_t) (argc - 1) * sizeof *file_names);
  if (file_names == nullptr) {
    goto file_names_error;
  }
  file_number = 0;
  for (int i = 1; i < argc; i++) {
    if (!compar_option(argv[i])) {
      file_names[file_number] = argv[i];
      file_number++;
    }
  }
  if (file_number < 2) {
    goto file_number_enough;
  }
  //-- BST INIT ----------------------------------------------------------------
  tree = bst_empty((int (*)(const void *, const void *))word_cell_compar);
  if (tree == nullptr) {
    goto tree_error_capacity;
  }
  ht = holdall_empty();
  if (ht == nullptr) {
    goto holdall_error_capacity;
  }
  for (size_t i = 0; i < file_number; ++i) {
    const char *fname = file_names[i];
    FILE *file = fopen(fname, "rb");
    if (file == nullptr) {
      fprintf(stderr, "%s: Can't open for reading file %s\n", argv[0], fname);
      goto error;
    }
    int c;
    size_t len;
    while ((c = fgetc(file)) != EOF) {
      if (isspace(c) && c != EOF) {
        c = fgetc(file);
      }
      if (feof(file)) {
        break;
      }
      size_t buffersize = MIN_CAPACITY;
      char *word = malloc(MIN_CAPACITY);
      if (word == nullptr) {
        fclose(file);
        goto malloc_word_error;
      }
      len = 0;
      while (c != EOF && !isspace(c)) {
        word[len] = (char) c;
        len++;
        if (len != 0 && len == maxlen) {
          while (!isspace(c) && c != EOF) {
            c = fgetc(file);
          }
          break;
        }
        if (len == buffersize) {
          buffersize *= MUL_CAPACITY;
          char *temp = realloc(word, buffersize);
          if (temp == nullptr) {
            free(word);
            fclose(file);
            goto realloc_error;
          }
          word = temp;
        }
        c = fgetc(file);
      }
      word[len] = '\0';
      void *wc = word_cell_empty(word, file_number);
      if (wc == nullptr) {
        free(word);
        fclose(file);
        goto word_cell_error;
      }
      app_to_file(wc, i);
      void *test_search = bst_search(tree, wc);
      if (test_search == nullptr) {
        void *add = bst_add_endofpath(tree, wc);
        if (add == nullptr) {
          free(word);
          free(wc);
          fclose(file);
          goto add_tree_error;
        }
        if (holdall_put(ht, wc) != 0) {
          free(word);
          free(wc);
          fclose(file);
          goto holdall_put_error;
        }
      } else {
        word_cell_dispo(wc);
        app_to_file(test_search, i);
      }
    }
    fclose(file);
  }
  //-- GRAPH REPR --------------------------------------------------------------
  if (graph) {
    printf("\t");
    for (size_t i = 0; i < file_number; i++) {
      printf("%s\t", file_names[i]);
    }
    printf("\n");
    bst_dft_infix_apply_context(tree, -4, &file_number,
        (int (*)(void *, const void *))jdis_graph, nullptr, nullptr);
    goto dispose;
  }
  //-- JDIS CALCULATION --------------------------------------------------------
  for (size_t i = 0; i < file_number; ++i) {
    for (size_t k = i + 1; k < file_number; k++) {
      double result = bst_jaccard(tree, (int) i, (int) k);
      printf("%.4f\t%s\t%s\n", result, file_names[i], file_names[k]);
    }
  }
  goto dispose;
  //-- GOTO
  // ----------------------------------------------------------------------
file_names_error:
  fprintf(stderr, "Memory allocation error.\n");
  goto error;
file_number_enough:
  fprintf(stderr, "Need at least two input files\n");
  goto error;
holdall_error_capacity:
  fprintf(stderr, "Error creating the holdall.\n");
  goto error;
tree_error_capacity:
  fprintf(stderr, "Error creating an empty tree.\n");
  goto error;
malloc_word_error:
  fprintf(stderr, "Error allocating memory for a word.\n");
  goto error;
add_tree_error:
  fprintf(stderr, "Error adding the word to the tree.\n");
  goto error;
holdall_put_error:
  fprintf(stderr, "Error adding the word to the holdall.\n");
  goto error;
word_cell_error:
  fprintf(stderr, "Error creating the word_cell.\n");
  goto error;
realloc_error:
  fprintf(stderr, "Error reallocating the word..\n");
  goto error;
error:
  r = EXIT_FAILURE;
  goto dispose;
dispose:
  if (ht != nullptr) {
    holdall_apply(ht, (int (*)(void *))word_cell_dispo);
    holdall_dispose(&ht);
  }
  if (tree != nullptr) {
    bst_dispose(&tree);
  }
  if (file_names != nullptr) {
    free(file_names);
  }
  return r;
}

bool compar_option(char *s) {
  return (s[0] == '-') && strlen(s) > 1;
}

void help() {
  printf("%s\n", USAGE_TEXT);
  printf("Computes Jaccard dissimilarities of sets of words in FILEs.\n\n");
  printf("For any pair of FILEs, dissimilarity"
      "is displayed first to four decimal places,\n");
  printf("followed by the two FILEs in the pair. A word is, "
      "by default, a maximum length\n");
  printf("sequence of characters that do not belong to the"
      "white-space characters set.\n\n");
  printf("Read the standard input for any FILE that is '-' on "
      "command line. The standard\n");
  printf("input is displayed as a pair of double "
      "quotation marks in productions.\n\n");
  printf("Program Information\n");
  printf("  -?, --help\n");
  printf("        Print this help message and exit.\n\n");
  printf("  --usage\n");
  printf("        Print a short usage message and exit.\n\n");
  printf("  --version\n");
  printf("        Print version information.\n\n");
  printf("Input Control\n");
  printf("  -i VALUE, --initial=VALUE\n");
  printf("        Set the maximal number of significant initial letters "
      "for words to VALUE. 0 means without\n");
  printf("        limitation. Default is 0.\n\n");
  printf("Output Control\n");
  printf("  -g, --graph\n");
  printf("        Suppress normal output. Instead, for each word found in "
      "any FILE, jdis list the FILEs in which it does or\n");
  printf("        does not appear. A header line indicates the FILE names: "
      "the name of the first FILE appears in the second\n");
  printf("        column, that of the second in the third, and so on. "
      "For the subsequent lines, a word appears in the first\n");
  printf("        column, followed by appearance marks: 'x' for yes, '-' "
      "for no. \n");
}
