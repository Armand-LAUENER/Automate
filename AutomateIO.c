#include "AutomateIO.h"
#include <stdarg.h>
#include <string.h>
#include <dirent.h>

// --- Helper for safe input ---
static void safe_gets(char *buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
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

// --- File Loading ---

bool loadAutomaton(const char *filename, Automaton *A, FILE *logFile) {
    memset(A, 0, sizeof(Automaton)); // Safety init

    FILE *file = fopen(filename, "r");
    if (!file) {
        logMessage(logFile, "Erreur : Impossible d'ouvrir %s\n", filename);
        return false;
    }

    int n_sym, n_states, n_init, n_final, n_trans;

    if (fscanf(file, "%d", &n_sym) != 1) goto error;
    if (fscanf(file, "%d", &n_states) != 1) goto error;

    if (!createAutomaton(A, n_states, n_sym)) goto error;

    if (fscanf(file, "%d", &n_init) != 1) goto error_cleanup;
    A->num_initials = n_init;
    A->initials = malloc(n_init * sizeof(int));
    if (n_init > 0 && !A->initials) goto error_cleanup;
    for (int i = 0; i < n_init; i++) {
        if (fscanf(file, "%d", &A->initials[i]) != 1) goto error_cleanup;
    }

    if (fscanf(file, "%d", &n_final) != 1) goto error_cleanup;
    A->num_finals = n_final;
    A->finals = malloc(n_final * sizeof(int));
    if (n_final > 0 && !A->finals) goto error_cleanup;
    for (int i = 0; i < n_final; i++) {
        if (fscanf(file, "%d", &A->finals[i]) != 1) goto error_cleanup;
    }

    if (fscanf(file, "%d", &n_trans) != 1) goto error_cleanup;
    for (int i = 0; i < n_trans; i++) {
        int u, v;
        char s;
        if (fscanf(file, "%d %c %d", &u, &s, &v) == 3) {
            addTransition(A, u, s - 'a', v);
        }
    }

    fclose(file);
    return true;

error_cleanup:
    freeAutomaton(A);
error:
    logMessage(logFile, "Erreur : Format de fichier invalide ou incomplet (%s).\n", filename);
    if (file) fclose(file);
    return false;
}

// --- Path Management ---

bool resolveAutomatesPath(char *buffer, size_t size, FILE *logFile) {
    const char *candidates[] = {
        "./Automates", "../Automates", "../../Automates", "../../../Automates"
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

void exportToDOT(const Automaton *A, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "digraph Automaton {\n");
    fprintf(file, "  rankdir=LR;\n");

    // States
    for (int i = 0; i < A->num_states; i++) {
        bool is_final = arrayContains(A->finals, A->num_finals, i);
        bool is_initial = arrayContains(A->initials, A->num_initials, i);
        fprintf(file, "  %d [label=\"%d\"", i, i);
        if (is_final) fprintf(file, ", shape=doublecircle");
        else fprintf(file, ", shape=circle");
        fprintf(file, "];\n");
        if (is_initial) {
            fprintf(file, "  start [shape=point];\n");
            fprintf(file, "  start -> %d;\n", i);
        }
    }

    // Transitions
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            int idx = i * A->num_symbols + j;
            TransitionList *tl = &A->transitions[idx];
            for (int k = 0; k < tl->count; k++) {
                fprintf(file, "  %d -> %d [label=\"%c\"];\n", i, tl->destinations[k], 'a' + j);
            }
        }
    }

    fprintf(file, "}\n");
    fclose(file);
}