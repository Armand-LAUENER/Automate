#ifndef AUTOMATE_ANALYSIS_H
#define AUTOMATE_ANALYSIS_H

#include "AutomateCore.h"
#include <stdio.h>

bool isDeterministic(const Automaton *A, FILE *logFile);
bool isStandard(const Automaton *A, FILE *logFile);
bool isComplete(const Automaton *A, FILE *logFile);
bool recognizeWord(const Automaton *A, const char *word, FILE *logFile);

#endif // AUTOMATE_ANALYSIS_H