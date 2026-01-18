# Projet Automate (Version Robuste & Modulaire)

Ce projet est une application en langage C permettant de **lire, analyser, transformer et tester des automates finis**. Il a été entièrement refactorisé pour être robuste, portable, modulaire et garanti sans fuite de mémoire.

## 📋 Fonctionnalités

### 1. Analyse et Robustesse
* **Lecture intelligente :** Le programme détecte automatiquement le dossier `Automates`, qu'il soit exécuté depuis la racine, un dossier de build (ex: `cmake-build-debug`), ou un autre sous-répertoire.
* **Allocation Dynamique :** Aucune limite arbitraire n'est imposée sur le nombre d'états ou de symboles.
* **Gestion de la mémoire :** Nettoyage automatique et rigoureux des ressources pour éviter toute fuite de mémoire (*memory leaks*).

### 2. Transformations Automatiques
* **Déterminisation :** Conversion d'un automate non-déterministe (AFN) vers un automate déterministe (AFD) via l'algorithme des sous-ensembles.
* **Standardisation :** Transformation pour obtenir un unique état initial sans transition entrante.
* **Complétion :** Ajout d'un état "poubelle" (puits) si nécessaire pour rendre l'automate complet.

### 3. Simulation
* **Reconnaissance de mots :** Permet de tester si des chaînes de caractères spécifiques sont acceptées ou rejetées par l'automate chargé.

### 4. Journalisation (Logging)
Tous les résultats d'analyse et de transformation sont sauvegardés pour traçabilité :
* Le fichier de log est nommé **`Exit.txt`**.
* Le programme tente d'écrire dans le dossier `Automates-exit/`.
* *Note :* Si ce dossier n'existe pas ou est inaccessible, le fichier sera créé à la racine de l'exécution.

## 📂 Structure du Projet

```text
.
├── AutomateCore.c      # Gestion mémoire et structures de base
├── AutomateCore.h
├── AutomateIO.c        # Entrées/Sorties (Fichiers & Logs)
├── AutomateIO.h
├── AutomateAnalysis.c  # Analyse (Déterminisme, Standard...)
├── AutomateAnalysis.h
├── AutomateTransform.c # Algorithmes de transformation
├── AutomateTransform.h
├── main.c              # Point d'entrée et menus
├── CMakeLists.txt      # Configuration de compilation CMake
├── Automates/          # Dossier contenant les fichiers d'entrée (.txt)
│   ├── #1.txt
│   └── ...
└── Automates-exit/     # Dossier de sortie
    └── Exit.txt        # Fichier de log généré
```
## 🛠️ Installation et Compilation

Pour utiliser ce projet, vous devez le compiler à partir des sources.

### Prérequis
* **CMake** (version 3.10 ou supérieure)
* **Compilateur C** compatible C11 (GCC, Clang, MSVC...)
* Un outil de build (Make, Ninja, Visual Studio...)

### Méthode 1 : Ligne de commande (Linux / macOS / Git Bash)

1.  **Cloner le dépôt :**
    ```bash
    git clone [https://github.com/armand-lauener/automate.git](https://github.com/armand-lauener/automate.git)
    cd automate
    ```

2.  **Créer un dossier de build et compiler :**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3.  **Lancer l'application :**
    ```bash
    ./Automate
    ```
    *(Le programme trouvera automatiquement le dossier `Automates` situé dans le dossier parent).*

### Méthode 2 : Avec un IDE (CLion, VS Code, Visual Studio)

1.  Ouvrez le dossier du projet dans votre IDE.
2.  L'IDE devrait détecter le fichier `CMakeLists.txt` et configurer le projet automatiquement.
3.  Sélectionnez la configuration **Automate** et cliquez sur **Build** (ou l'icône marteau).
4.  Cliquez sur **Run** (ou la flèche verte) pour lancer le programme.

## 📝 Format des fichiers Automates

Les fichiers `.txt` placés dans le dossier `Automates/` doivent respecter le format suivant (les espaces et sauts de ligne sont ignorés) :

1.  Nombre de symboles
2.  Nombre d'états
3.  Nombre d'états initiaux + Liste des états initiaux
4.  Nombre d'états terminaux + Liste des états terminaux
5.  Nombre de transitions
6.  Liste des transitions (État Départ - Symbole - État Arrivée)

### Exemple de fichier

```text
2           <-- 2 Symboles (a, b)
3           <-- 3 États (0, 1, 2)
1 0         <-- 1 état initial : l'état 0
1 2         <-- 1 état terminal : l'état 2
4           <-- 4 transitions
0 a 1
0 b 0
1 a 2
2 b 2
```
## 👤 Auteur
Projet développé par Armand Lauener et Nazim Mekideche.