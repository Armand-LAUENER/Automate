#include "AutomateTransform.h"
#include "AutomateIO.h" // For logMessage if needed
#include <string.h>

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