#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <ctype.h>
#include "Automate.h"

#define BUFFER_SIZE 1024
#define DEFAULT_CAPACITY 2

// --- Fonctions utilitaires privées ---

static void safe_gets(char *buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

static bool arrayContains(const int *array, int size, int value) {
    for (int i = 0; i < size; i++) {
        if (array[i] == value) return true;
    }
    return false;
}

static void addUnique(int **array, int *size, int value) {
    if (!arrayContains(*array, *size, value)) {
        int *temp = realloc(*array, (*size + 1) * sizeof(int));
        if (!temp) {
            perror("Erreur fatale: Plus de memoire (realloc)");
            exit(EXIT_FAILURE);
        }
        *array = temp;
        (*array)[*size] = value;
        (*size)++;
    }
}

// --- Logique de recherche du dossier ---

static bool resolveAutomatesPath(char *buffer, size_t size, FILE *logFile) {
    const char *candidates[] = {
        "./Automates",
        "../Automates",
        "../../Automates",
        "../../../Automates"
    };

    for (int i = 0; i < 4; i++) {
        DIR *dir = opendir(candidates[i]);
        if (dir) {
            closedir(dir);
            strncpy(buffer, candidates[i], size - 1);
            buffer[size - 1] = '\0';
            return true;
        }
    }

    logMessage(logFile, "Erreur critique : Impossible de trouver le dossier 'Automates'.\n");
    return false;
}

// --- Gestion de la mémoire ---

Automaton createAutomaton(int num_states, int num_symbols) {
    Automaton A;
    A.num_states = num_states;
    A.num_symbols = num_symbols;
    A.num_initials = 0;
    A.initials = NULL;
    A.num_finals = 0;
    A.finals = NULL;

    int total_cells = num_states * num_symbols;
    A.transitions = malloc(total_cells * sizeof(TransitionList));

    if (!A.transitions) {
        perror("Erreur fatale: Allocation memoire automate");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < total_cells; i++) {
        A.transitions[i].count = 0;
        A.transitions[i].capacity = 0;
        A.transitions[i].destinations = NULL;
    }
    return A;
}

void freeAutomaton(Automaton *A) {
    // Vérification de pointeur NULL pour robustesse
    if (!A) return;

    if (A->initials) { free(A->initials); A->initials = NULL; }
    if (A->finals) { free(A->finals); A->finals = NULL; }

    if (A->transitions) {
        int total_cells = A->num_states * A->num_symbols;
        for (int i = 0; i < total_cells; i++) {
            if (A->transitions[i].destinations) {
                free(A->transitions[i].destinations);
            }
        }
        free(A->transitions);
        A->transitions = NULL;
    }
    // Remise à zéro pour éviter utilisation après libération
    A->num_states = 0;
    A->num_symbols = 0;
}

void addTransition(Automaton *A, int from, int symbol_idx, int to) {
    if (from < 0 || from >= A->num_states || symbol_idx < 0 || symbol_idx >= A->num_symbols) return;

    int index = from * A->num_symbols + symbol_idx;
    TransitionList *list = &A->transitions[index];

    if (list->count >= list->capacity) {
        int new_cap = (list->capacity == 0) ? DEFAULT_CAPACITY : list->capacity * 2;
        // Sécurisation du realloc
        int *temp = realloc(list->destinations, new_cap * sizeof(int));
        if (!temp) {
            perror("Erreur critique: Echec realloc transitions");
            // On ne quitte pas forcément, mais on n'ajoute pas la transition
            return;
        }
        list->destinations = temp;
        list->capacity = new_cap;
    }

    list->destinations[list->count++] = to;
}

// --- Logging ---

void logMessage(FILE *logFile, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    if (logFile) {
        va_start(args, format);
        vfprintf(logFile, format, args);
        va_end(args);
        fflush(logFile);
    }
}

void printAutomaton(const Automaton *A, FILE *logFile) {
    logMessage(logFile, "Alphabet : ");
    for (int i = 0; i < A->num_symbols; i++)
        logMessage(logFile, "%c ", 'a' + i);

    logMessage(logFile, "\nEtats : %d", A->num_states);

    logMessage(logFile, "\nEtats initiaux : ");
    for (int i = 0; i < A->num_initials; i++)
        logMessage(logFile, "%d ", A->initials[i]);

    logMessage(logFile, "\nEtats terminaux : ");
    for (int i = 0; i < A->num_finals; i++)
        logMessage(logFile, "%d ", A->finals[i]);

    logMessage(logFile, "\nTransitions :\n");
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            int idx = i * A->num_symbols + j;
            TransitionList *list = &A->transitions[idx];
            for (int k = 0; k < list->count; k++) {
                logMessage(logFile, "  %d --(%c)--> %d\n", i, 'a' + j, list->destinations[k]);
            }
        }
    }
    logMessage(logFile, "-------------------------\n");
}

// --- Lecture de Fichier ---

bool loadAutomaton(const char *filename, Automaton *A, FILE *logFile) {
    // Initialisation préventive pour éviter free sur garbage en cas d'erreur
    memset(A, 0, sizeof(Automaton));

    FILE *file = fopen(filename, "r");
    if (!file) {
        logMessage(logFile, "Erreur : Impossible d'ouvrir %s\n", filename);
        return false;
    }

    int n_sym, n_states, n_init, n_final, n_trans;

    // Utilisation d'une macro ou goto pour nettoyer en cas d'erreur
    if (fscanf(file, "%d", &n_sym) != 1) goto error;
    if (fscanf(file, "%d", &n_states) != 1) goto error;

    *A = createAutomaton(n_states, n_sym);

    if (fscanf(file, "%d", &n_init) != 1) goto error_with_cleanup;
    A->num_initials = n_init;
    A->initials = malloc(n_init * sizeof(int));
    if (n_init > 0 && !A->initials) goto error_with_cleanup;
    for (int i = 0; i < n_init; i++) {
        if (fscanf(file, "%d", &A->initials[i]) != 1) goto error_with_cleanup;
    }

    if (fscanf(file, "%d", &n_final) != 1) goto error_with_cleanup;
    A->num_finals = n_final;
    A->finals = malloc(n_final * sizeof(int));
    if (n_final > 0 && !A->finals) goto error_with_cleanup;
    for (int i = 0; i < n_final; i++) {
        if (fscanf(file, "%d", &A->finals[i]) != 1) goto error_with_cleanup;
    }

    if (fscanf(file, "%d", &n_trans) != 1) goto error_with_cleanup;
    for (int i = 0; i < n_trans; i++) {
        int u, v;
        char s;
        if (fscanf(file, "%d %c %d", &u, &s, &v) == 3) {
            addTransition(A, u, s - 'a', v);
        }
    }

    fclose(file);
    return true;

error_with_cleanup:
    freeAutomaton(A); // CORRECTION CRITIQUE: Libération de la mémoire partielle

error:
    logMessage(logFile, "Erreur : Format de fichier invalide ou incomplet (%s).\n", filename);
    if (file) fclose(file);
    return false;
}

// --- Analyse & Transformations ---

bool isDeterministic(const Automaton *A, FILE *logFile) {
    if (A->num_initials != 1) return false;
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            if (A->transitions[i * A->num_symbols + j].count > 1) return false;
        }
    }
    return true;
}

bool isComplete(const Automaton *A, FILE *logFile) {
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            if (A->transitions[i * A->num_symbols + j].count == 0) return false;
        }
    }
    return true;
}

bool isStandard(const Automaton *A, FILE *logFile) {
    if (A->num_initials != 1) return false;
    int init = A->initials[0];
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            TransitionList *list = &A->transitions[i * A->num_symbols + j];
            for (int k = 0; k < list->count; k++) {
                if (list->destinations[k] == init) return false;
            }
        }
    }
    return true;
}

Automaton complete(const Automaton *A, FILE *logFile) {
    Automaton newA = createAutomaton(A->num_states + 1, A->num_symbols);
    int trashState = A->num_states;

    newA.num_initials = A->num_initials;
    newA.initials = malloc(newA.num_initials * sizeof(int));
    memcpy(newA.initials, A->initials, newA.num_initials * sizeof(int));

    newA.num_finals = A->num_finals;
    newA.finals = malloc(newA.num_finals * sizeof(int));
    memcpy(newA.finals, A->finals, newA.num_finals * sizeof(int));

    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            int idx = i * A->num_symbols + j;
            if (A->transitions[idx].count == 0) {
                addTransition(&newA, i, j, trashState);
            } else {
                for(int k=0; k < A->transitions[idx].count; k++) {
                    addTransition(&newA, i, j, A->transitions[idx].destinations[k]);
                }
            }
        }
    }
    for (int j = 0; j < A->num_symbols; j++) addTransition(&newA, trashState, j, trashState);
    return newA;
}

Automaton standardize(const Automaton *A, FILE *logFile) {
    Automaton newA = createAutomaton(A->num_states + 1, A->num_symbols);
    int newInit = A->num_states;

    newA.num_initials = 1;
    newA.initials = malloc(sizeof(int));
    newA.initials[0] = newInit;

    for(int i=0; i<A->num_states; i++) {
        for(int j=0; j<A->num_symbols; j++) {
            int idx = i * A->num_symbols + j;
            for(int k=0; k<A->transitions[idx].count; k++) {
                addTransition(&newA, i, j, A->transitions[idx].destinations[k]);
            }
        }
    }

    newA.num_finals = A->num_finals;
    newA.finals = malloc((A->num_finals + 1) * sizeof(int));
    memcpy(newA.finals, A->finals, A->num_finals * sizeof(int));

    bool initIsFinal = false;
    for(int i=0; i<A->num_initials; i++) {
        if (arrayContains(A->finals, A->num_finals, A->initials[i])) {
            initIsFinal = true; break;
        }
    }
    if (initIsFinal) newA.finals[newA.num_finals++] = newInit;

    for(int j=0; j<A->num_symbols; j++) {
        for(int i=0; i<A->num_initials; i++) {
            int oldInit = A->initials[i];
            int idx = oldInit * A->num_symbols + j;
            for(int k=0; k<A->transitions[idx].count; k++) {
                int dest = A->transitions[idx].destinations[k];
                int newIdx = newInit * A->num_symbols + j;
                bool exists = false;
                for(int t=0; t<newA.transitions[newIdx].count; t++) {
                    if(newA.transitions[newIdx].destinations[t] == dest) exists = true;
                }
                if(!exists) addTransition(&newA, newInit, j, dest);
            }
        }
    }
    return newA;
}

typedef struct { int *states; int count; } Subset;
static bool areSubsetsEqual(Subset *a, Subset *b) {
    if (a->count != b->count) return false;
    for (int i=0; i<a->count; i++) {
        bool found = false;
        for (int j=0; j<b->count; j++) {
            if (a->states[i] == b->states[j]) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

Automaton determinize(const Automaton *A, FILE *logFile) {
    int capacity = 64;
    Subset *subsets = malloc(capacity * sizeof(Subset));
    int num_subsets = 0;
    int **tempTrans = malloc(capacity * sizeof(int*));
    for(int i=0; i<capacity; i++) {
        tempTrans[i] = malloc(A->num_symbols * sizeof(int));
        for(int s=0; s<A->num_symbols; s++) tempTrans[i][s] = -1;
    }

    Subset initSubset = {NULL, 0};
    for (int i=0; i<A->num_initials; i++) addUnique(&initSubset.states, &initSubset.count, A->initials[i]);
    subsets[num_subsets++] = initSubset;

    int processed = 0;
    while (processed < num_subsets) {
        if (num_subsets >= capacity - 1) {
            capacity *= 2;
            // Sécurisation basique pour l'exercice (dans un vrai projet, utiliser var temp)
            subsets = realloc(subsets, capacity * sizeof(Subset));
            tempTrans = realloc(tempTrans, capacity * sizeof(int*));
            for(int k=num_subsets; k<capacity; k++) {
                tempTrans[k] = malloc(A->num_symbols * sizeof(int));
                for(int s=0; s<A->num_symbols; s++) tempTrans[k][s] = -1;
            }
        }

        Subset current = subsets[processed];
        for (int sym = 0; sym < A->num_symbols; sym++) {
            Subset target = {NULL, 0};
            for (int k = 0; k < current.count; k++) {
                int state = current.states[k];
                int idx = state * A->num_symbols + sym;
                TransitionList *tl = &A->transitions[idx];
                for (int t = 0; t < tl->count; t++) {
                    addUnique(&target.states, &target.count, tl->destinations[t]);
                }
            }

            if (target.count > 0) {
                int foundIdx = -1;
                for (int existing = 0; existing < num_subsets; existing++) {
                    if (areSubsetsEqual(&target, &subsets[existing])) {
                        foundIdx = existing; break;
                    }
                }
                if (foundIdx == -1) {
                    foundIdx = num_subsets;
                    subsets[num_subsets++] = target;
                } else {
                    free(target.states);
                }
                tempTrans[processed][sym] = foundIdx;
            } else {
                 free(target.states);
            }
        }
        processed++;
    }

    Automaton newA = createAutomaton(num_subsets, A->num_symbols);
    newA.num_initials = 1;
    newA.initials = malloc(sizeof(int));
    newA.initials[0] = 0;

    for(int i=0; i<num_subsets; i++) {
        bool isFinal = false;
        for(int k=0; k<subsets[i].count; k++) {
            if (arrayContains(A->finals, A->num_finals, subsets[i].states[k])) {
                isFinal = true; break;
            }
        }
        if (isFinal) {
            newA.finals = realloc(newA.finals, (newA.num_finals + 1) * sizeof(int));
            newA.finals[newA.num_finals++] = i;
        }
        for(int sym=0; sym<A->num_symbols; sym++) {
            if (tempTrans[i][sym] != -1) {
                addTransition(&newA, i, sym, tempTrans[i][sym]);
            }
        }
        free(subsets[i].states);
        free(tempTrans[i]);
    }
    free(subsets);
    free(tempTrans);
    return newA;
}

bool recognizeWord(const Automaton *A, const char *word, FILE *logFile) {
    if (A->num_initials == 0) return false;
    // Robustesse: Si plus de 1 état initial, c'est non-déterministe
    if (A->num_initials > 1) {
        logMessage(logFile, "Erreur de logique: Automate non-deterministe (plusieurs etats initiaux).\n");
        return false;
    }

    int current = A->initials[0];

    for (int i = 0; word[i] != '\0'; i++) {
        int sym = word[i] - 'a';
        if (sym < 0 || sym >= A->num_symbols) return false;

        int idx = current * A->num_symbols + sym;

        // Robustesse: Vérification du déterminisme strict
        if (A->transitions[idx].count > 1) {
            logMessage(logFile, "Attention: Ambiguite detectee (non-determinisme) a l'etat %d. Le mot est rejete par securite.\n", current);
            return false;
        }

        if (A->transitions[idx].count == 0) return false;
        current = A->transitions[idx].destinations[0];
    }
    return arrayContains(A->finals, A->num_finals, current);
}

void processAutomaton(const char *filepath, FILE *logFile) {
    Automaton A;
    // loadAutomaton gère maintenant le cleanup interne en cas d'erreur partielle
    if (!loadAutomaton(filepath, &A, logFile)) return;

    logMessage(logFile, "\n=== Analyse de : %s ===\n", filepath);
    printAutomaton(&A, logFile);

    if (!isDeterministic(&A, logFile)) {
        logMessage(logFile, "\n>>> Transformation : Determinisation\n");
        Automaton det = determinize(&A, logFile);
        freeAutomaton(&A); A = det;
        printAutomaton(&A, logFile);
    }
    if (!isStandard(&A, logFile)) {
        logMessage(logFile, "\n>>> Transformation : Standardisation\n");
        Automaton std = standardize(&A, logFile);
        freeAutomaton(&A); A = std;
        printAutomaton(&A, logFile);
    }
    if (!isComplete(&A, logFile)) {
        logMessage(logFile, "\n>>> Transformation : Completion\n");
        Automaton comp = complete(&A, logFile);
        freeAutomaton(&A); A = comp;
        printAutomaton(&A, logFile);
    }

    while (1) {
        char buffer[256];
        logMessage(logFile, "\nTester un mot ? (entrez le mot ou 'vide' ou tapez Entree pour passer) : ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL || buffer[0] == '\n') break;
        buffer[strcspn(buffer, "\n")] = 0;
        char *word = buffer;
        if (strcmp(word, "vide") == 0) word = "";

        if (recognizeWord(&A, word, logFile)) {
            logMessage(logFile, "Resultat : '%s' est ACCEPTE.\n", word);
        } else {
            logMessage(logFile, "Resultat : '%s' est REFUSE.\n", word);
        }
    }
    freeAutomaton(&A);
}

// --- UTILISATION DU PATH RESOLVER ---

void listAndChooseFile(char *buffer, size_t size, FILE *logFile) {
    char folderPath[512];

    if (!resolveAutomatesPath(folderPath, sizeof(folderPath), logFile)) {
        buffer[0] = '\0';
        return;
    }

    DIR *d = opendir(folderPath);
    if (!d) return;

    struct dirent *dir;
    char files[100][256];
    int count = 0;

    logMessage(logFile, "\nFichiers disponibles (dans %s) :\n", folderPath);
    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".txt")) {
            strncpy(files[count], dir->d_name, 255);
            logMessage(logFile, "%d. %s\n", count + 1, files[count]);
            count++;
            if (count >= 100) break;
        }
    }
    closedir(d);

    if (count == 0) {
        logMessage(logFile, "Aucun fichier .txt trouve.\n");
        return;
    }

    int choice = 0;
    do {
        printf("Votre choix (1-%d) : ", count);
        char input[10];
        safe_gets(input, sizeof(input));
        choice = atoi(input);
    } while (choice < 1 || choice > count);

    snprintf(buffer, size, "%s/%s", folderPath, files[choice - 1]);
}

void processAllAutomata(FILE *logFile) {
    char folderPath[512];
    if (!resolveAutomatesPath(folderPath, sizeof(folderPath), logFile)) return;

    DIR *d = opendir(folderPath);
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".txt")) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", folderPath, dir->d_name);
            processAutomaton(path, logFile);
        }
    }
    closedir(d);
}