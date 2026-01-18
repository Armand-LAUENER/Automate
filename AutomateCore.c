#include "AutomateCore.h"
#include <string.h>

#define DEFAULT_CAPACITY 2

// --- Utilities ---

bool arrayContains(const int *array, int size, int value) {
    for (int i = 0; i < size; i++) {
        if (array[i] == value) return true;
    }
    return false;
}

void addUnique(int **array, int *size, int value) {
    if (!arrayContains(*array, *size, value)) {
        int *temp = realloc(*array, (*size + 1) * sizeof(int));
        if (!temp) {
            perror("Fatal Error: Out of memory (realloc in addUnique)");
            exit(EXIT_FAILURE);
        }
        *array = temp;
        (*array)[*size] = value;
        (*size)++;
    }
}

// --- Memory Management ---

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
        perror("Fatal Error: Memory allocation for automaton transitions failed");
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
    A->num_states = 0;
    A->num_symbols = 0;
}

void addTransition(Automaton *A, int from, int symbol_idx, int to) {
    if (from < 0 || from >= A->num_states || symbol_idx < 0 || symbol_idx >= A->num_symbols) return;

    int index = from * A->num_symbols + symbol_idx;
    TransitionList *list = &A->transitions[index];

    if (list->count >= list->capacity) {
        int new_cap = (list->capacity == 0) ? DEFAULT_CAPACITY : list->capacity * 2;
        int *temp = realloc(list->destinations, new_cap * sizeof(int));
        if (!temp) {
            perror("Critical Error: Failed to realloc transition list");
            return; 
        }
        list->destinations = temp;
        list->capacity = new_cap;
    }
    list->destinations[list->count++] = to;
}