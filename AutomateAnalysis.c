#include "AutomateAnalysis.h"
#include "AutomateIO.h" // Needed for logMessage

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

bool recognizeWord(const Automaton *A, const char *word, FILE *logFile) {
    if (A->num_initials == 0) return false;
    
    // Safety check for non-determinism
    if (A->num_initials > 1) {
        logMessage(logFile, "Erreur logique: Impossible de simuler un automate non-deterministe.\n");
        return false;
    }

    int current = A->initials[0]; 

    for (int i = 0; word[i] != '\0'; i++) {
        int sym = word[i] - 'a';
        if (sym < 0 || sym >= A->num_symbols) return false;
        
        int idx = current * A->num_symbols + sym;
        
        if (A->transitions[idx].count > 1) {
            logMessage(logFile, "Attention: Ambiguite detectee (non-determinisme) a l'etat %d.\n", current);
            return false;
        }
        
        if (A->transitions[idx].count == 0) return false;
        current = A->transitions[idx].destinations[0]; 
    }
    return arrayContains(A->finals, A->num_finals, current);
}