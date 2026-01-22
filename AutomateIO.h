#ifndef AUTOMATE_IO_H
#define AUTOMATE_IO_H

#include <stdio.h>
#include "AutomateCore.h"

// --- Logging ---
void logMessage(FILE *logFile, const char *format, ...);
void printAutomaton(const Automaton *A, FILE *logFile);

// --- File Operations ---
bool loadAutomaton(const char *filename, Automaton *A, FILE *logFile);
void listAndChooseFile(char *buffer, size_t size, FILE *logFile);
bool resolveAutomatesPath(char *buffer, size_t size, FILE *logFile);
void exportToDOT(const Automaton *A, const char *filename);

#endif // AUTOMATE_IO_H