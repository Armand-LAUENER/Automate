#ifndef AUTOMATE_H
#define AUTOMATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

#define MAX_FILES 100
#define MAX_STATES 100
#define MAX_SYMBOLS 26
#define MAX_TRANSITIONS_PER_CELL 10
#define MAX_NEW_STATES 1000


// Pointeur global pour le fichier de sortie.
extern FILE *outputFile;

// Structure pour stocker plusieurs transitions pour un même symbole
typedef struct {
    int nb;                             // Nombre de transitions enregistrées pour ce symbole
    int dest[MAX_TRANSITIONS_PER_CELL]; // États d'arrivée
} TransitionList;

// Pour la déterminisation (représentation d'un sous-ensemble d'états)
typedef struct {
    int states[MAX_STATES]; // 1 si l'état est présent, 0 sinon
} Subset;

typedef struct {
    int num_symbols;                // Nombre de symboles de l'alphabet
    int num_states;                 // Nombre d'états
    int num_initial_states;         // Nombre d'états initiaux
    int initial_states[MAX_STATES]; // Liste des états initiaux
    int num_final_states;           // Nombre d'états terminaux
    int final_states[MAX_STATES];   // Liste des états terminaux
    TransitionList transitions[MAX_STATES][MAX_SYMBOLS]; // Table des transitions
} Automate;

// Fonctions utilitaires de gestion de fichiers
void choisirFichier(char *filename);

// Fonctions de lecture et d'affichage de l'automate
void lireAutomateGeneral(const char *filename, Automate *A);
void afficherAutomateGeneral(const Automate *A);
void log_printf(const char *format, ...);
void processAutomateFromFile(const char *filename);
void processAllAutomates();

// Fonctions d'analyse et de transformation
int estDeterministe(const Automate *A);
int estStandard(const Automate *A);
int estComplet(const Automate *A);
Automate determiniser(const Automate *A);
Automate standardiser(const Automate *A);
Automate completer(const Automate A);

// Fonction de reconnaissance de mot
int reconnaitreMot(const Automate *A, const char *mot);

// Fonction utilitaire pour comparer deux sous-ensembles
int sameSubset(int *subset1, int *subset2, int n);

#endif
