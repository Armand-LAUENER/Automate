
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <libgen.h>
#include <limits.h>
#include "Automate.h"

/*-------------------------------------------
  Fonctions utilitaires de gestion de fichiers
-------------------------------------------*/

// Vérifie si un fichier est vide
int estFichierVide(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_printf("Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        return 1;
    }
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (!isspace(c)) {
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    return 1;
}

// Fonction pour obtenir le chemin du dossier Automates
char *getAutomatesPath() {
    static char directoryPath[PATH_MAX];
    char sourcePath[PATH_MAX];

    // Obtenir le chemin absolu du fichier source sous Windows
    if (_fullpath(sourcePath, __FILE__, sizeof(sourcePath)) == NULL) {
        perror("Erreur lors de la récupération du chemin absolu");
        return NULL;
    }

    // Extraire le dossier contenant le fichier source
    char *dirPath = dirname(sourcePath);

    // Construire le chemin vers Automates
    snprintf(directoryPath, sizeof(directoryPath), "%s/Automates", dirPath);

    return directoryPath;
}


// Liste les fichiers .txt et permet de choisir un fichier
void choisirFichier(char *filename) {
    char *path = getAutomatesPath();
    struct dirent *entry;
    DIR *dir = opendir(path);
    char files[MAX_FILES][256];
    int count = 0;
    char automatePerso[256] = "";
    if (!dir) {
        log_printf("Erreur : Impossible d'ouvrir le dossier %s\n", path);
        exit(EXIT_FAILURE);
    }
    log_printf("Fichiers disponibles :\n");
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".txt") != NULL) {
            if (strcmp(entry->d_name, "AutomatePerso.txt") == 0) {
                strcpy(automatePerso, entry->d_name);
            } else {
                strcpy(files[count], entry->d_name);
                count++;
            }
        }
    }
    closedir(dir);
    for (int i = 0; i < count; i++)
        log_printf("%d. %s\n", i + 1, files[i]);
    if (strlen(automatePerso) > 0) {
        log_printf("%d. %s\n", count + 1, automatePerso);
        strcpy(files[count], automatePerso);
        count++;
    }
    int choix;
    do {
        log_printf("Choisissez un fichier (1-%d) : ", count);
        scanf("%d", &choix);
    } while (choix < 1 || choix > count);
    snprintf(filename, 256, "%s/%s", path, files[choix - 1]);
    log_printf("Vous avez choisi : %s \n", filename);
}

/*-------------------------------------------
  Fonctions de lecture et d'affichage de l'automate
-------------------------------------------*/

// Initialise la table des transitions à nb = 0
static void initAutomate(Automate *A) {
    for (int i = 0; i < MAX_STATES; i++) {
        for (int j = 0; j < MAX_SYMBOLS; j++) {
            A->transitions[i][j].nb = 0;
        }
    }
}

// Lit un automate général à partir d'un fichier texte
void lireAutomateGeneral(const char *filename, Automate *A) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_printf("Erreur : impossible d'ouvrir le fichier %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%d", &A->num_symbols) != 1) {
        log_printf("Erreur de lecture du nombre de symboles.\n");
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%d", &A->num_states) != 1) {
        log_printf("Erreur de lecture du nombre d'états.\n");
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%d", &A->num_initial_states) != 1) {
        log_printf("Erreur de lecture du nombre d'états initiaux.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < A->num_initial_states; i++) {
        if (fscanf(file, "%d", &A->initial_states[i]) != 1) {
            log_printf("Erreur de lecture de l'état initial numéro %d.\n", i);
            exit(EXIT_FAILURE);
        }
    }

    if (fscanf(file, "%d", &A->num_final_states) != 1) {
        log_printf("Erreur de lecture du nombre d'états terminaux.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < A->num_final_states; i++) {
        if (fscanf(file, "%d", &A->final_states[i]) != 1) {
            log_printf("Erreur de lecture de l'état terminal numéro %d.\n", i);
            exit(EXIT_FAILURE);
        }
    }

    initAutomate(A);

    int num_transitions;
    if (fscanf(file, "%d", &num_transitions) != 1) {
        log_printf("Erreur de lecture du nombre de transitions.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_transitions; i++) {
        int from, to;
        char symbol;
        if (fscanf(file, "%d %c %d", &from, &symbol, &to) != 3) {
            log_printf("Erreur de lecture de la transition numéro %d.\n", i);
            exit(EXIT_FAILURE);
        }
        int sym_index = symbol - 'a';
        if (sym_index < 0 || sym_index >= A->num_symbols) {
            log_printf("Symbole invalide '%c' dans la transition %d.\n", symbol, i);
            continue;
        }
        int nb = A->transitions[from][sym_index].nb;
        if (nb < MAX_TRANSITIONS_PER_CELL) {
            A->transitions[from][sym_index].dest[nb] = to;
            A->transitions[from][sym_index].nb++;
        } else {
            log_printf("Erreur : trop de transitions pour l'état %d et le symbole '%c'.\n", from, symbol);
        }
    }
    fclose(file);
}

// Affiche l'automate en utilisant log_printf (écriture sur terminal et fichier)
void afficherAutomateGeneral(const Automate *A) {
    log_printf("Alphabet : ");
    for (int i = 0; i < A->num_symbols; i++)
        log_printf("%c ", 'a' + i);
    log_printf("\nNombre d'états : %d\n", A->num_states);

    log_printf("États initiaux : ");
    for (int i = 0; i < A->num_initial_states; i++)
        log_printf("%d ", A->initial_states[i]);
    log_printf("\nÉtats terminaux : ");
    for (int i = 0; i < A->num_final_states; i++)
        log_printf("%d ", A->final_states[i]);
    log_printf("\nTransitions :\n");
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            if (A->transitions[i][j].nb > 0) {
                for (int k = 0; k < A->transitions[i][j].nb; k++) {
                    log_printf("%d --(%c)--> %d\n", i, 'a' + j, A->transitions[i][j].dest[k]);
                }
            }
        }
    }
}

// Fonction log_printf : écrit sur le terminal et dans le fichier de sortie.
void log_printf(const char *format, ...) {
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);

    vprintf(format, args);

    if (outputFile != NULL) {
        vfprintf(outputFile, format, args_copy);
        fflush(outputFile);
    }
    va_end(args);
    va_end(args_copy);
}

// Traitement d'un automate à partir d'un fichier.
void processAutomateFromFile(const char *filename) {
    Automate A, A_det, A_std, A_comp;

    lireAutomateGeneral(filename, &A);
    log_printf("\nAutomate original (fichier : %s) :\n", filename);
    afficherAutomateGeneral(&A);

    // Vérification avant déterminisation
    if (!estDeterministe(&A)) {
        log_printf("\nDéterminisation en cours...\n");
        A_det = determiniser(&A);
        log_printf("Automate déterminisé :\n");
        afficherAutomateGeneral(&A_det);
    } else {
        log_printf("\nL'automate est déjà déterministe. Pas besoin de déterminiser.\n");
        A_det = A;
    }

    // Vérification avant standardisation
    if (!estStandard(&A_det)) {
        log_printf("\nStandardisation en cours...\n");
        A_std = standardiser(&A_det);
        log_printf("Automate standardisé :\n");
        afficherAutomateGeneral(&A_std);
    } else {
        log_printf("\nL'automate est déjà standard. Pas besoin de standardiser.\n");
        A_std = A_det;
    }

    // Vérification avant complétion
    if (!estComplet(&A_std)) {
        log_printf("\nComplétion en cours...\n");
        A_comp = completer(A_std);
        log_printf("Automate complété :\n");
        afficherAutomateGeneral(&A_comp);
    } else {
        log_printf("\nL'automate est déjà complet. Pas besoin de compléter.\n");
        A_comp = A_std;
    }

    // Boucle de test de reconnaissance de mots
    char reponse[10];
    do {
        log_printf("\nVoulez-vous tester un mot sur cet automate ? (o/n) : ");
        scanf("%s", reponse);
        if (reponse[0] == 'o' || reponse[0] == 'O') {
            char mot[256];
            log_printf("Entrez le mot à tester : ");
            scanf("%s", mot);
            if (reconnaitreMot(&A_comp, mot))
                log_printf("Le mot \"%s\" est reconnu par l'automate.\n", mot);
            else
                log_printf("Le mot \"%s\" n'est pas reconnu par l'automate.\n", mot);
        }
    } while (reponse[0] == 'o' || reponse[0] == 'O');
}


// Traitement de tous les automates dans le dossier.
void processAllAutomates() {
    char *path = getAutomatesPath();
    char files[MAX_FILES][256];
    int count = 0;
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        log_printf("Erreur : impossible d'ouvrir le dossier %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".txt") != NULL) {
            strcpy(files[count], entry->d_name);
            count++;
        }
    }
    closedir(dir);

    if (count == 0) {
        log_printf("Aucun fichier automate trouvé dans le répertoire.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s%s", path, files[i]);
        log_printf("\n=== Traitement de l'automate : %s ===\n", filepath);
        processAutomateFromFile(filepath);
    }
}

/*-------------------------------------------
  Fonctions d'analyse et de transformation de l'automate
-------------------------------------------*/

int estDeterministe(const Automate *A) {
    if (A->num_initial_states != 1) {
        log_printf("Erreur : l'automate doit avoir un seul état initial pour être déterministe.\n");
        return 0;
    }
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            if (A->transitions[i][j].nb > 1) {
                log_printf("Non-déterminisme détecté : état %d, symbole '%c' a %d transitions.\n", i, 'a' + j, A->transitions[i][j].nb);
                return 0;
            }
            if (A->transitions[i][j].nb == 1) {
                int dest = A->transitions[i][j].dest[0];
                if (dest < 0 || dest >= A->num_states) {
                    log_printf("Transition invalide : de l'état %d avec le symbole '%c' vers l'état %d.\n", i, 'a' + j, dest);
                    return 0;
                }
            }
        }
    }
    return 1;
}

int estStandard(const Automate *A) {
    if (A->num_initial_states != 1) {
        log_printf("L'automate n'est pas standard : il doit avoir un seul état initial.\n");
        return 0;
    }
    int init = A->initial_states[0];
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            for (int k = 0; k < A->transitions[i][j].nb; k++) {
                if (A->transitions[i][j].dest[k] == init) {
                    log_printf("L'automate n'est pas standard : l'état initial %d est atteint par une transition.\n", init);
                    return 0;
                }
            }
        }
    }
    return 1;
}

int estComplet(const Automate *A) {
    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            if (A->transitions[i][j].nb == 0) {
                log_printf("L'automate n'est pas complet : aucune transition pour l'état %d avec le symbole '%c'.\n", i, 'a' + j);
                return 0;
            }
        }
    }
    return 1;
}

int sameSubset(int *subset1, int *subset2, int n) {
    for (int i = 0; i < n; i++) {
        if (subset1[i] != subset2[i])
            return 0;
    }
    return 1;
}

// Déterminisation (construction par sous-ensembles)
Automate determiniser(const Automate *A) {
    Automate newA; // Automate déterminisé à retourner
    int newTransitions[MAX_NEW_STATES][MAX_SYMBOLS]; // Tableau des nouvelles transitions
    Subset newSubsets[MAX_NEW_STATES]; // Tableau des nouveaux sous-ensembles d'états
    int newStateCount = 0, sinkIndex = -1; // Nombre d'états créés et index de l'état puits

    // Initialisation des structures de données
    for (int i = 0; i < MAX_NEW_STATES; i++) {
        for (int j = 0; j < A->num_symbols; j++)
            newTransitions[i][j] = -1; // Initialisation des transitions à -1 (pas de transition)
        for (int j = 0; j < MAX_STATES; j++)
            newSubsets[i].states[j] = 0; // Initialisation des sous-ensembles d'états à 0
    }

    // Ajout des états initiaux dans le premier sous-ensemble
    for (int i = 0; i < A->num_initial_states; i++) {
        newSubsets[0].states[A->initial_states[i]] = 1;
    }
    newStateCount = 1; // Le premier état est ajouté

    // Parcours des nouveaux états créés
    for (int i = 0; i < newStateCount; i++) {
        for (int sym = 0; sym < A->num_symbols; sym++) { // Pour chaque symbole de l'alphabet
            int unionSubset[MAX_STATES] = {0}; // Ensemble des états atteignables

            // Construction du sous-ensemble accessible
            for (int s = 0; s < A->num_states; s++) {
                if (newSubsets[i].states[s] == 1) { // Si l'état fait partie du sous-ensemble courant
                    for (int k = 0; k < A->transitions[s][sym].nb; k++) {
                        int dest = A->transitions[s][sym].dest[k]; // Destination de la transition
                        if (dest != -1)
                            unionSubset[dest] = 1; // Ajout de l'état destination
                    }
                }
            }

            // Vérifier si ce sous-ensemble existe déjà
            int foundIndex = -1;
            for (int j = 0; j < newStateCount; j++) {
                if (sameSubset(newSubsets[j].states, unionSubset, A->num_states)) {
                    foundIndex = j; // L'état existe déjà
                    break;
                }
            }

            // Si le sous-ensemble est nouveau, on l'ajoute
            if (foundIndex == -1) {
                int isEmpty = 1;
                for (int k = 0; k < A->num_states; k++) {
                    if (unionSubset[k] != 0) {
                        isEmpty = 0;
                        break;
                    }
                }
                if (isEmpty) { // Gestion de l'état puits
                    if (sinkIndex != -1)
                        foundIndex = sinkIndex;
                    else {
                        for (int k = 0; k < A->num_states; k++)
                            newSubsets[newStateCount].states[k] = unionSubset[k];
                        foundIndex = newStateCount;
                        sinkIndex = newStateCount;
                        newStateCount++;
                    }
                } else {
                    for (int k = 0; k < A->num_states; k++)
                        newSubsets[newStateCount].states[k] = unionSubset[k];
                    foundIndex = newStateCount;
                    newStateCount++;
                }
            }
            newTransitions[i][sym] = foundIndex; // Mise à jour de la transition
        }
    }

    // Initialisation du nouvel automate
    newA.num_symbols = A->num_symbols;
    newA.num_states = newStateCount;
    newA.num_initial_states = 1;
    newA.initial_states[0] = 0; // L'état initial est toujours le premier sous-ensemble

    // Remplissage des transitions dans le nouvel automate
    for (int i = 0; i < newStateCount; i++) {
        for (int sym = 0; sym < A->num_symbols; sym++) {
            newA.transitions[i][sym].nb = (newTransitions[i][sym] != -1) ? 1 : 0;
            if (newTransitions[i][sym] != -1)
                newA.transitions[i][sym].dest[0] = newTransitions[i][sym];
        }
    }

    // Détermination des états finaux
    newA.num_final_states = 0;
    for (int i = 0; i < newStateCount; i++) {
        int isFinal = 0;
        for (int s = 0; s < A->num_states; s++) {
            if (newSubsets[i].states[s] == 1) { // Vérifier si un état de l'ensemble est final
                for (int f = 0; f < A->num_final_states; f++) {
                    if (A->final_states[f] == s) {
                        isFinal = 1;
                        break;
                    }
                }
            }
            if (isFinal)
                break;
        }
        if (isFinal) {
            newA.final_states[newA.num_final_states] = i; // Marquer l'état comme final
            newA.num_final_states++;
        }
    }
    return newA; // Retourner l'automate déterminisé
}


Automate standardiser(const Automate *A) {
    Automate newA;

    // Ajout d'un nouvel état initial supplémentaire
    int oldStates = A->num_states;
    newA.num_states = oldStates + 1; // On ajoute un état supplémentaire
    newA.num_symbols = A->num_symbols;

    // Définition du nouvel état initial
    newA.num_initial_states = 1;
    newA.initial_states[0] = oldStates; // Le nouvel état initial est l'état ajouté

    // Copie des transitions de l'automate d'origine
    for (int i = 0; i < oldStates; i++) {
        for (int j = 0; j < A->num_symbols; j++)
            newA.transitions[i][j] = A->transitions[i][j]; // Copie des transitions existantes
    }

    // Ajout des transitions depuis le nouvel état initial
    if (A->num_initial_states > 0) {
        for (int j = 0; j < A->num_symbols; j++)
            newA.transitions[oldStates][j] = A->transitions[A->initial_states[0]][j];
    }

    // Copie des états finaux de l'automate d'origine
    newA.num_final_states = A->num_final_states;
    for (int i = 0; i < A->num_final_states; i++)
        newA.final_states[i] = A->final_states[i];

    // Si un des anciens états initiaux était aussi un état final,
    // alors le nouvel état initial devient aussi un état final
    for (int i = 0; i < A->num_initial_states; i++) {
        for (int j = 0; j < A->num_final_states; j++) {
            if (A->initial_states[i] == A->final_states[j]) {
                newA.final_states[newA.num_final_states] = oldStates; // On ajoute le nouvel état initial comme état final
                newA.num_final_states++;
                break;
            }
        }
    }

    return newA;
}


Automate completer(const Automate A) {

    Automate newA =A; // Copie de l'automate d'origine dans newA

    int etatPoubelle = -1; // Définition du nouvel état poubelle (dernier indice disponible)



    // Ajout des transitions vers l'etat poubelle pour les états ayant des transitions manquantes
    for (int i = 0; i < newA.num_states; i++) {
        for (int j = 0; j < newA.num_symbols; j++) {
            if (newA.transitions[i][j].nb == 0) { // Si une transition est absente
                newA.transitions[i][j].nb = 1; // On ajoute une transition
                newA.transitions[i][j].dest[0] = etatPoubelle; // Elle mène à l'état poubelle
            }
        }
    }
    newA.num_states++; // Augmentation du nombre d'états pour inclure l'etat poubelle

    // Initialisation des transitions de l'état poubelle
    for (int i = 0; i < newA.num_symbols; i++) {
        newA.transitions[etatPoubelle][i].nb = 1; // Une seule transition
        newA.transitions[etatPoubelle][i].dest[0] = etatPoubelle; // Qui boucle sur lui-même
    }


    log_printf("Automate complété avec l'état poubelle.\n");
    return newA; // Retourne l'automate complété
}




int reconnaitreMot(const Automate *A, const char *mot) {
    // Initialisation de l'état courant à l'état initial de l'automate
    int current_state = A->initial_states[0];

    // Parcours du mot caractère par caractère
    for (int i = 0; mot[i] != '\0'; i++) {
        char c = mot[i];

        // Ignorer les espaces, retours à la ligne et retours chariot
        if (c == '\n' || c == '\r' || c == ' ')
            continue;

        // Conversion du caractère en indice correspondant dans l'alphabet de l'automate
        int index = c - 'a';

        // Vérification si le caractère est hors de l'alphabet de l'automate
        if (index < 0 || index >= A->num_symbols) {
            log_printf("Le symbole '%c' n'est pas dans l'alphabet.\n", c);
            return 0; // Mot rejeté
        }

        // Vérification s'il existe une transition depuis l'état courant avec ce symbole
        if (A->transitions[current_state][index].nb == 0)
            return 0; // Aucune transition possible, mot rejeté

        // Passage à l'état suivant (on suppose ici une transition déterministe)
        current_state = A->transitions[current_state][index].dest[0];
    }

    // Vérification si l'état final atteint est un état acceptant
    for (int i = 0; i < A->num_final_states; i++) {
        if (current_state == A->final_states[i])
            return 1; // Mot accepté
    }

    return 0; // Mot rejeté
}
