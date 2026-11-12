# jdis — dissimilarité de Jaccard entre documents texte, en C

`jdis` lit plusieurs fichiers texte, extrait l'ensemble des mots de chacun et calcule, pour chaque paire de fichiers, leur **dissimilarité de Jaccard**. Les mots sont indexés dans un **arbre binaire de recherche équilibré (AVL)** que j'ai implémenté sans bibliothèque externe.

Projet universitaire de L2 Informatique (Université de Rouen Normandie, mai 2025). Le code est publié **tel qu'il a été rendu**, sans correction a posteriori. Le défaut connu est décrit dans [Limitations connues](#limitations-connues).

```console
$ ./jdis chat.txt chien.txt souris.txt
0.5714	chat.txt	chien.txt
0.0000	chat.txt	souris.txt
0.5714	chien.txt	souris.txt
```

## Sommaire

- [La distance de Jaccard](#la-distance-de-jaccard)
- [Fonctionnement](#fonctionnement)
- [Structures de données](#structures-de-données)
- [Organisation des fichiers](#organisation-des-fichiers)
- [Compilation](#compilation)
- [Utilisation](#utilisation)
- [Vérifications](#vérifications)
- [Limitations connues](#limitations-connues)
- [Contexte universitaire](#contexte-universitaire)

## La distance de Jaccard

Pour deux ensembles finis $A$ et $B$, l'**indice de Jaccard** mesure la part d'éléments communs :

$$J(A, B) = \frac{|A \cap B|}{|A \cup B|}$$

La **dissimilarité** (ou distance) de Jaccard en est le complément :

$$d_J(A, B) = 1 - \frac{|A \cap B|}{|A \cup B|}$$

Elle vaut 0 pour deux ensembles identiques et 1 pour deux ensembles disjoints. Ici, chaque document est réduit à l'**ensemble** de ses mots : l'ordre et le nombre d'occurrences sont ignorés.

**Exemple.** « le chat mange la souris » et « le chien mange la viande » :

- $A = \{\text{chat, la, le, mange, souris}\}$, $B = \{\text{chien, la, le, mange, viande}\}$
- $A \cap B = \{\text{la, le, mange}\}$, donc 3 mots communs ; $A \cup B$ compte 7 mots
- $d_J = 1 - 3/7 \approx 0{,}5714$

## Fonctionnement

1. **Lecture des options**, puis des noms de fichiers (au moins deux).
2. **Découpage en mots.** Chaque fichier est lu caractère par caractère (`fgetc`). Un mot est une suite de caractères qui ne sont pas des espaces au sens de `isspace` (espace, tabulation, saut de ligne…). Le mot est accumulé dans un tampon dynamique qui double de taille si nécessaire (16, 32, 64… octets).
3. **Indexation.** Pour chaque mot, le programme cherche dans l'arbre une cellule de même texte :
   - si elle existe, il marque la présence du mot dans le fichier courant ;
   - sinon, il insère une nouvelle cellule, avec un tableau de booléens (un par fichier), et l'enregistre dans un « fourre-tout » qui servira à la libération finale.
4. **Calcul.** Pour chaque paire de fichiers $(i, k)$, un parcours infixe de l'arbre compte les mots présents dans $i$ ou $k$ (union) et dans les deux (intersection). Une fonction de rappel et un contexte passés au parcours font ce comptage.
5. **Affichage** de $d_J$ avec 4 décimales, suivi des deux noms de fichiers. Avec `-g`, le programme affiche à la place le tableau d'appartenance des mots, triés.
6. **Libération** de toute la mémoire.

### Complexité

Notations : $N$ le nombre total de mots lus, $n$ le nombre de mots distincts, $P$ le nombre de fichiers, $L$ la longueur d'un mot.

| Étape | Temps | Commentaire |
|---|---|---|
| Indexation | $O(N \cdot L \log n)$ | recherche dans un AVL de hauteur $O(\log n)$, comparaison de chaînes en $O(L)$ |
| Calcul des distances | $O(P^2 \cdot n)$ | un parcours complet de l'arbre par paire de fichiers |
| Mémoire | $O(n \cdot (L + P))$ | une cellule par mot distinct, avec son tableau de $P$ booléens |

## Structures de données

| Structure | Fichiers | Rôle |
|---|---|---|
| **Arbre binaire de recherche équilibré (AVL)**, générique (`void *`) | `bst/` | Dictionnaire des mots distincts, ordonnés par `strcoll`. |
| **Cellule de mot** `word_cell` | `jdis/` | Pointeur vers le mot et membre de tableau flexible `bool app[]` : `app[i]` vaut vrai si le mot apparaît dans le fichier `i`. |
| **Fourre-tout** `holdall`, liste simplement chaînée | `holdall/` | Conserve les cellules allouées pour les libérer en fin d'exécution (module fourni par l'équipe pédagogique). |

Détails sur l'arbre (`bst/bst.c`) :

- **Rééquilibrage AVL** après chaque insertion et suppression : rotations simples et doubles, hauteur et équilibre stockés dans chaque nœud. La hauteur reste donc logarithmique, même si les mots arrivent triés.
- Chaque nœud stocke aussi la **taille de son sous-arbre**, ce qui permet `bst_size` en temps constant et le calcul du **rang** d'un élément.
- **Parcours infixe générique** `bst_dft_infix_apply_context`, dans les deux sens, avec un contexte et des fonctions de rappel : le calcul de Jaccard et l'affichage `-g` reposent sur ce parcours.
- Le module implémente une interface complète (recherche, ajout, suppression par remontée gauche, hauteur, distance, numéro, rang, représentation graphique), dont une partie seulement sert à `jdis`.

## Organisation des fichiers

```
.
├── bst/            arbre binaire de recherche (AVL)
│   ├── bst.h         interface (fournie par l'équipe pédagogique)
│   ├── bst.c         implémentation
│   └── bst_ip.h      complexités de l'implémentation
├── holdall/        fourre-tout (fourni par l'équipe pédagogique)
│   ├── holdall.h
│   ├── holdall.c
│   └── holdall_ip.h
├── jdis/           cellules de mots et calcul de Jaccard
│   ├── jdis.h
│   └── jdis.c
└── main/           programme principal
    ├── main.c        options, lecture des fichiers, affichage
    └── makefile
```

## Compilation

Prérequis : un compilateur **C23** (le code utilise `nullptr` et `bool` sans en-tête), comme GCC ≥ 13, et `make`. Testé avec GCC 13.3 sous Linux (Ubuntu 24.04).

```sh
make -C main          # produit main/jdis
make -C main clean
```

Le makefile compile avec `-std=c2x -Wall -Wextra -Wpedantic -Wconversion -Wwrite-strings -Werror -O2`. Le projet compile **sans aucun avertissement** avec ces options.

## Utilisation

```
Usage: jdis [OPTION]... FILE1 FILE2 [FILE]...
```

| Option | Effet |
|---|---|
| `-g`, `--graph` | Afficher le tableau d'appartenance des mots au lieu des distances. |
| `-?`, `--help` | Afficher l'aide complète. |
| `--usage` | Afficher la ligne d'usage. |
| `-version` | Afficher la version. |
| `-i N`, `--initial=N` | Prévue par le sujet (troncature des mots) ; non implémentée dans cette version. |

### Exemples

Créer trois fichiers de test :

```sh
printf 'le chat mange la souris\n'  > chat.txt
printf 'le chien mange la viande\n' > chien.txt
printf 'la souris mange le chat\n'  > souris.txt
```

Calculer les distances pour toutes les paires (`chat.txt` et `souris.txt` contiennent les mêmes mots dans un autre ordre) :

```console
$ ./jdis chat.txt chien.txt souris.txt
0.5714	chat.txt	chien.txt
0.0000	chat.txt	souris.txt
0.5714	chien.txt	souris.txt
```

Afficher le tableau d'appartenance (`x` : présent, `-` : absent) :

```console
$ ./jdis -g chat.txt chien.txt
	chat.txt	chien.txt
chat	x	-
chien	-	x
la	x	x
le	x	x
mange	x	x
souris	x	-
viande	-	x
```

## Vérifications

Vérifications faites lors de la préparation de ce dépôt (novembre 2026), avec GCC 13.3 sous Linux :

- **Compilation** sans aucun avertissement avec les options strictes du makefile.
- **Résultats** : sur les exemples ci-dessus, les sorties correspondent aux valeurs calculées à la main (par exemple $1 - 3/7 = 0{,}5714$).
- **Mémoire** : Valgrind (`--leak-check=full`) ne signale aucune fuite ni aucun accès invalide sur ces exemples. Le programme a aussi été exécuté avec AddressSanitizer et UndefinedBehaviorSanitizer.
- **Cas limites** : ces tests ont révélé le défaut décrit ci-dessous. J'ai choisi de le documenter plutôt que de modifier le code rendu.

## Limitations connues

**Défaut connu**

- **Espaces consécutifs** : un seul caractère d'espacement est sauté avant chaque mot. Trois espaces consécutifs, plusieurs espaces en début de fichier ou plusieurs lignes vides font compter un mot vide `""`, ce qui fausse la distance. Par exemple, `"a\n\n\nb"` comparé à `"a b"` donne 0,3333 au lieu de 0.

**Choix de conception et périmètre**

- **Définition simple du mot** : seuls les espaces séparent les mots. La ponctuation fait partie du mot (`chat.` ≠ `chat`) et la casse est respectée (`Le` ≠ `le`).
- **Allocations par occurrence** : chaque mot lu provoque deux allocations (le mot et sa cellule), libérées aussitôt s'il est déjà connu. C'est simple et sûr, mais coûteux sur de gros volumes.

## Contexte universitaire

- **Formation** : Licence Informatique, 2ᵉ année (semestre 4), Université de Rouen Normandie.
- **UE** : Algorithmique 3 — [Structures de données arborescentes](https://formation.univ-rouen.fr/fr/catalogue-de-l-offre-de-formation/licence-lmd-XA/licence-informatique-L5MDAST8/licence-informatique-L5MDASW3/ue1-structures-de-donnees-arborescentes-L5U674W3.html).
- **Date** : mai 2025.
- **Auteur** : Salim Allami. Le code a été écrit sans assistance d'IA.
- **Code fourni par l'équipe pédagogique** : l'interface `bst/bst.h` et le module `holdall/` font partie du sujet. Le programme `jdis`, l'implémentation de l'arbre (`bst/bst.c`) et le module `jdis/` sont mon travail.

## Droits

© 2025 Salim Allami. Tous droits réservés. Les fichiers fournis par l'équipe pédagogique de l'Université de Rouen Normandie restent la propriété de leurs auteurs.
