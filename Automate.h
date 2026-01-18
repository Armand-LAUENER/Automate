#ifndef AUTOMATE_H
#define AUTOMATE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Dynamic Structures ---

// Transition list for a cell (state, symbol)
typedef struct {
    int count;          // Number of transitions
    int capacity;       // Allocated capacity
    int *destinations;  // Dynamic array of destination states
} TransitionList;

// Main automaton structure
typedef struct {
    int num_symbols;    // Alphabet size
    int num_states;     // Total number of states

    int num_initials;
    int *initials;      // Dynamic array of initial states

    int num_finals;
    int *finals;        // Dynamic array of final states

    // Flattened 1D transition table for efficiency
    // Access: transitions[state * num_symbols + symbol]
    TransitionList *transitions;
} Automaton;

// --- Memory Management ---
Automaton createAutomaton(int num_states, int num_symbols);
void freeAutomaton(Automaton *A);
void addTransition(Automaton *A, int from, int symbol_idx, int to);

// --- Input / Output (Logging) ---
void logMessage(FILE *logFile, const char *format, ...);
void printAutomaton(const Automaton *A, FILE *logFile);
bool loadAutomaton(const char *filename, Automaton *A, FILE *logFile);
void listAndChooseFile(char *buffer, size_t size, FILE *logFile);

// --- Analysis ---
bool isDeterministic(const Automaton *A, FILE *logFile);
bool isStandard(const Automaton *A, FILE *logFile);
bool isComplete(const Automaton *A, FILE *logFile);

// --- Transformations ---
Automaton determinize(const Automaton *A, FILE *logFile);
Automaton standardize(const Automaton *A, FILE *logFile);
Automaton complete(const Automaton *A, FILE *logFile);

// --- Simulation ---
bool recognizeWord(const Automaton *A, const char *word, FILE *logFile);

// --- High-level Processing ---
void processAutomaton(const char *filepath, FILE *logFile);
void processAllAutomata(FILE *logFile);

#endif // AUTOMATE_H