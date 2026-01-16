#include <stdio.h>
#include "Automate.h"

// Définition du pointeur global pour le fichier de sortie.
FILE *outputFile = NULL;

int main() {
    int choix;
    char filename[256];

    // Ouverture du fichier de sortie dans le dossier "Sortie".
    outputFile = fopen("Sortie.txt", "w");
    if (outputFile == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        exit(EXIT_FAILURE);
    }

    do {
        printf("\n=== Menu Principal ===\n");
        printf("1. Lire et traiter un automate existant\n");
        printf("2. Lire et traiter tous les automates\n");
        printf("3. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                choisirFichier(filename);
                processAutomateFromFile(filename);
                break;
            case 2:
                processAllAutomates();
                break;
            case 3:
                printf("Au revoir !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }
    } while (choix != 3);

    fclose(outputFile);  // Fermeture du fichier de sortie.
    return 0;
}
