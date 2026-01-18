#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "AutomateCore.h"
#include "AutomateIO.h"
#include "AutomateAnalysis.h"
#include "AutomateTransform.h"

// --- Helper Local ---

void processAutomaton(const char *filepath, FILE *logFile) {
    Automaton A;
    if (!loadAutomaton(filepath, &A, logFile)) return;

    logMessage(logFile, "\n=== Analyse de : %s ===\n", filepath);
    printAutomaton(&A, logFile);

    if (!isDeterministic(&A, logFile)) {
        logMessage(logFile, "\n>>> Transformation : Determinisation\n");
        Automaton det = determinize(&A, logFile);
        freeAutomaton(&A); A = det;
        printAutomaton(&A, logFile);
    }
    if (!isStandard(&A, logFile)) {
        logMessage(logFile, "\n>>> Transformation : Standardisation\n");
        Automaton std = standardize(&A, logFile);
        freeAutomaton(&A); A = std;
        printAutomaton(&A, logFile);
    }
    if (!isComplete(&A, logFile)) {
        logMessage(logFile, "\n>>> Transformation : Completion\n");
        Automaton comp = complete(&A, logFile);
        freeAutomaton(&A); A = comp;
        printAutomaton(&A, logFile);
    }

    while (1) {
        char buffer[256];
        logMessage(logFile, "\nTester un mot ? (entrez le mot ou 'vide' ou tapez Entree pour passer) : ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL || buffer[0] == '\n') break;
        buffer[strcspn(buffer, "\n")] = 0;
        char *word = buffer;
        if (strcmp(word, "vide") == 0) word = "";

        if (recognizeWord(&A, word, logFile)) {
            logMessage(logFile, "Resultat : '%s' est ACCEPTE.\n", word);
        } else {
            logMessage(logFile, "Resultat : '%s' est REFUSE.\n", word);
        }
    }
    freeAutomaton(&A);
}

void processAllAutomata(FILE *logFile) {
    char folderPath[512];
    if (!resolveAutomatesPath(folderPath, sizeof(folderPath), logFile)) return;

    DIR *d = opendir(folderPath);
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".txt")) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", folderPath, dir->d_name);
            processAutomaton(path, logFile);
        }
    }
    closedir(d);
}

static void resolveOutputPath(char *buffer, size_t size) {
    const char *targetFolder = "Automates-exit";
    const char *candidates[] = { ".", "..", "../..", "../../.." };

    buffer[0] = '\0';
    for (int i = 0; i < 4; i++) {
        char testPath[512];
        snprintf(testPath, sizeof(testPath), "%s/%s", candidates[i], targetFolder);
        DIR *dir = opendir(testPath);
        if (dir) {
            closedir(dir);
            snprintf(buffer, size, "%s/Exit.txt", testPath);
            return;
        }
    }
    snprintf(buffer, size, "./%s/Exit.txt", targetFolder);
}

int main() {
    char outputPath[512];
    resolveOutputPath(outputPath, sizeof(outputPath));
    FILE *logFile = fopen(outputPath, "w");

    if (!logFile) {
        logFile = fopen("Exit.txt", "w"); // Fallback
    }

    if (logFile) printf("Log file initialized: %s\n", outputPath);

    logMessage(logFile, "=== Projet Automate (Modulaire) ===\n");

    int choice = 0;
    char buffer[10];

    do {
        logMessage(logFile, "\n--- MENU PRINCIPAL ---\n");
        logMessage(logFile, "1. Traiter un automate specifique\n");
        logMessage(logFile, "2. Traiter tous les automates\n");
        logMessage(logFile, "3. Quitter\n");
        printf("Choix : ");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        choice = atoi(buffer);

        char filepath[512];
        switch (choice) {
            case 1:
                listAndChooseFile(filepath, sizeof(filepath), logFile);
                if (filepath[0] != '\0') processAutomaton(filepath, logFile);
                break;
            case 2:
                processAllAutomata(logFile);
                break;
            case 3:
                logMessage(logFile, "Fermeture du programme.\n");
                break;
            default:
                logMessage(logFile, "Choix invalide.\n");
        }
    } while (choice != 3);

    if (logFile) fclose(logFile);
    return 0;
}