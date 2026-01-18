#ifndef AUTOMATE_TRANSFORM_H
#define AUTOMATE_TRANSFORM_H

#include "AutomateCore.h"
#include <stdio.h>

Automaton determinize(const Automaton *A, FILE *logFile);
Automaton standardize(const Automaton *A, FILE *logFile);
Automaton complete(const Automaton *A, FILE *logFile);

#endif // AUTOMATE_TRANSFORM_H