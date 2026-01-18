# Automata Project (Robust & Modular Version)

This project is a C application designed to **read, analyze, transform, and test finite automata**. It has been entirely refactored to be robust, portable, modular, and guaranteed memory-leak-free.

## 📋 Features

### 1. Analysis and Robustness
* **Smart Reading:** The program automatically detects the `Automates` folder, whether executed from the root, a build folder (e.g., `cmake-build-debug`), or another subdirectory.
* **Dynamic Allocation:** No arbitrary limits are imposed on the number of states or symbols.
* **Memory Management:** Automatic and rigorous resource cleanup to prevent any memory leaks.

### 2. Automatic Transformations
* **Determinization:** Conversion from Non-deterministic Finite Automaton (NFA) to Deterministic Finite Automaton (DFA) via the subset construction algorithm.
* **Standardization:** Transformation to create a single initial state with no incoming transitions.
* **Completion:** Addition of a "garbage" (sink) state if necessary to make the automaton complete.

### 3. Simulation
* **Word Recognition:** Allows testing whether specific strings are accepted or rejected by the loaded automaton.

### 4. Logging
All analysis and transformation results are saved for traceability:
* The log file is named **`Exit.txt`**.
* The program attempts to write to the `Automates-exit/` folder.
* *Note:* If this folder does not exist or is inaccessible, the file will be created at the execution root.

## 📂 Project Structure

```text
.
├── AutomateCore.c      # Memory management and base structures
├── AutomateCore.h
├── AutomateIO.c        # Input/Output (Files & Logs)
├── AutomateIO.h
├── AutomateAnalysis.c  # Analysis (Determinism, Standard...)
├── AutomateAnalysis.h
├── AutomateTransform.c # Transformation algorithms
├── AutomateTransform.h
├── main.c              # Entry point and menus
├── CMakeLists.txt      # CMake build configuration
├── Automates/          # Folder containing input files (.txt)
│   ├── #1.txt
│   └── ...
└── Automates-exit/     # Output folder
    └── Exit.txt        # Generated log file
```
## 🛠️ Installation and Compilation

To use this project, you must compile it from source.

### Prerequisites
* **CMake** (version 3.10 or higher)
* **C11 compatible C Compiler** (GCC, Clang, MSVC...)
* A build tool (Make, Ninja, Visual Studio...)

### Method 1: Command Line (Linux / macOS / Git Bash)

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/armand-lauener/automate.git](https://github.com/armand-lauener/automate.git)
    cd automate
    ```

2.  **Create a build folder and compile:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3.  **Run the application:**
    ```bash
    ./Automate
    ```
    *(The program will automatically find the `Automates` folder located in the parent directory).*

### Method 2: With an IDE (CLion, VS Code, Visual Studio)

1.  Open the project folder in your IDE.
2.  The IDE should detect the `CMakeLists.txt` file and configure the project automatically.
3.  Select the **Automate** configuration and click **Build** (or the hammer icon).
4.  Click **Run** (or the green arrow) to launch the program.

## 📝 Automaton File Format

The `.txt` files placed in the `Automates/` folder must respect the following format (spaces and newlines are ignored):

1.  Number of symbols
2.  Number of states
3.  Number of initial states + List of initial states
4.  Number of terminal states + List of terminal states
5.  Number of transitions
6.  List of transitions (Start State - Symbol - End State)

### File Example

```text
2           <-- 2 Symbols (a, b)
3           <-- 3 States (0, 1, 2)
1 0         <-- 1 initial state: state 0
1 2         <-- 1 terminal state: state 2
4           <-- 4 transitions
0 a 1
0 b 0
1 a 2
2 b 2
```
## 👤 Authors
Project developed by Armand Lauener and Nazim Mekideche.