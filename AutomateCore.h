#ifndef AUTOMATE_CORE_H
#define AUTOMATE_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Dynamic Structures ---

typedef struct {
    int count;          // Number of transitions
    int capacity;       // Allocated capacity
    int *destinations;  // Dynamic array of destination states
} TransitionList;

typedef struct {
    int num_symbols;    // Alphabet size
    int num_states;     // Total number of states

    int num_initials;
    int *initials;      // Dynamic array of initial states

    int num_finals;
    int *finals;        // Dynamic array of final states

    // Flattened 1D transition table for efficiency
    TransitionList *transitions;
} Automaton;

// --- Memory Management ---
Automaton createAutomaton(int num_states, int num_symbols);
void freeAutomaton(Automaton *A);
void addTransition(Automaton *A, int from, int symbol_idx, int to);

// --- Utilities ---
bool arrayContains(const int *array, int size, int value);
void addUnique(int **array, int *size, int value);

#endif // AUTOMATE_CORE_H