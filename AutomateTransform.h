#ifndef AUTOMATE_TRANSFORM_H
#define AUTOMATE_TRANSFORM_H

#include "AutomateCore.h"
#include <stdio.h>

bool determinize(const Automaton *A, Automaton *out, FILE *logFile);
bool standardize(const Automaton *A, Automaton *out, FILE *logFile);
bool complete(const Automaton *A, Automaton *out, FILE *logFile);
bool minimize(const Automaton *A, Automaton *out, FILE *logFile);

#endif // AUTOMATE_TRANSFORM_H