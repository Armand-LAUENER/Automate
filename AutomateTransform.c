#include "AutomateTransform.h"
#include "AutomateIO.h" // For logMessage if needed
#include <string.h>

bool complete(const Automaton *A, Automaton *out, FILE *logFile) {
    if (!createAutomaton(out, A->num_states + 1, A->num_symbols)) return false;
    int trashState = A->num_states;

    out->num_initials = A->num_initials;
    out->initials = malloc(out->num_initials * sizeof(int));
    if (out->num_initials > 0 && !out->initials) {
        freeAutomaton(out);
        return false;
    }
    memcpy(out->initials, A->initials, out->num_initials * sizeof(int));

    out->num_finals = A->num_finals;
    out->finals = malloc(out->num_finals * sizeof(int));
    if (out->num_finals > 0 && !out->finals) {
        freeAutomaton(out);
        return false;
    }
    memcpy(out->finals, A->finals, out->num_finals * sizeof(int));

    for (int i = 0; i < A->num_states; i++) {
        for (int j = 0; j < A->num_symbols; j++) {
            int idx = i * A->num_symbols + j;
            if (A->transitions[idx].count == 0) {
                if (!addTransition(out, i, j, trashState)) {
                    freeAutomaton(out);
                    return false;
                }
            } else {
                for(int k=0; k < A->transitions[idx].count; k++) {
                    if (!addTransition(out, i, j, A->transitions[idx].destinations[k])) {
                        freeAutomaton(out);
                        return false;
                    }
                }
            }
        }
    }
    for (int j = 0; j < A->num_symbols; j++) {
        if (!addTransition(out, trashState, j, trashState)) {
            freeAutomaton(out);
            return false;
        }
    }
    return true;
}

bool standardize(const Automaton *A, Automaton *out, FILE *logFile) {
    if (!createAutomaton(out, A->num_states + 1, A->num_symbols)) return false;
    int newInit = A->num_states;

    out->num_initials = 1;
    out->initials = malloc(sizeof(int));
    if (!out->initials) {
        freeAutomaton(out);
        return false;
    }
    out->initials[0] = newInit;

    for(int i=0; i<A->num_states; i++) {
        for(int j=0; j<A->num_symbols; j++) {
            int idx = i * A->num_symbols + j;
            for(int k=0; k<A->transitions[idx].count; k++) {
                if (!addTransition(out, i, j, A->transitions[idx].destinations[k])) {
                    freeAutomaton(out);
                    return false;
                }
            }
        }
    }

    out->num_finals = A->num_finals;
    out->finals = malloc((A->num_finals + 1) * sizeof(int));
    if (!out->finals) {
        freeAutomaton(out);
        return false;
    }
    memcpy(out->finals, A->finals, A->num_finals * sizeof(int));

    bool initIsFinal = false;
    for(int i=0; i<A->num_initials; i++) {
        if (arrayContains(A->finals, A->num_finals, A->initials[i])) {
            initIsFinal = true; break;
        }
    }
    if (initIsFinal) out->finals[out->num_finals++] = newInit;

    for(int j=0; j<A->num_symbols; j++) {
        for(int i=0; i<A->num_initials; i++) {
            int oldInit = A->initials[i];
            int idx = oldInit * A->num_symbols + j;
            for(int k=0; k<A->transitions[idx].count; k++) {
                int dest = A->transitions[idx].destinations[k];
                int newIdx = newInit * A->num_symbols + j;
                bool exists = false;
                for(int t=0; t<out->transitions[newIdx].count; t++) {
                    if(out->transitions[newIdx].destinations[t] == dest) exists = true;
                }
                if(!exists) {
                    if (!addTransition(out, newInit, j, dest)) {
                        freeAutomaton(out);
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

typedef struct { int *states; int count; } Subset;

static bool areSubsetsEqual(Subset *a, Subset *b) {
    if (a->count != b->count) return false;
    for (int i=0; i<a->count; i++) {
        bool found = false;
        for (int j=0; j<b->count; j++) {
            if (a->states[i] == b->states[j]) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

bool determinize(const Automaton *A, Automaton *out, FILE *logFile) {
    int capacity = 64; 
    Subset *subsets = malloc(capacity * sizeof(Subset));
    if (!subsets) return false;
    int num_subsets = 0;
    int **tempTrans = malloc(capacity * sizeof(int*));
    if (!tempTrans) {
        free(subsets);
        return false;
    }
    for(int i=0; i<capacity; i++) {
        tempTrans[i] = malloc(A->num_symbols * sizeof(int));
        if (!tempTrans[i]) {
            // cleanup
            for(int j=0; j<i; j++) free(tempTrans[j]);
            free(tempTrans);
            free(subsets);
            return false;
        }
        for(int s=0; s<A->num_symbols; s++) tempTrans[i][s] = -1;
    }

    Subset initSubset = {NULL, 0};
    for (int i=0; i<A->num_initials; i++) addUnique(&initSubset.states, &initSubset.count, A->initials[i]);
    subsets[num_subsets++] = initSubset;

    int processed = 0;
    while (processed < num_subsets) {
        if (num_subsets >= capacity - 1) {
            capacity *= 2;
            subsets = realloc(subsets, capacity * sizeof(Subset));
            if (!subsets) {
                // cleanup
                for(int i=0; i<num_subsets; i++) free(subsets[i].states);
                for(int i=0; i<capacity/2; i++) free(tempTrans[i]);
                free(tempTrans);
                return false;
            }
            tempTrans = realloc(tempTrans, capacity * sizeof(int*));
            if (!tempTrans) {
                free(subsets);
                return false;
            }
            for(int k=num_subsets; k<capacity; k++) {
                tempTrans[k] = malloc(A->num_symbols * sizeof(int));
                if (!tempTrans[k]) {
                    // cleanup
                    for(int j=0; j<k; j++) free(tempTrans[j]);
                    free(tempTrans);
                    free(subsets);
                    return false;
                }
                for(int s=0; s<A->num_symbols; s++) tempTrans[k][s] = -1;
            }
        }

        Subset current = subsets[processed];
        for (int sym = 0; sym < A->num_symbols; sym++) {
            Subset target = {NULL, 0};
            for (int k = 0; k < current.count; k++) {
                int state = current.states[k];
                int idx = state * A->num_symbols + sym;
                TransitionList *tl = &A->transitions[idx];
                for (int t = 0; t < tl->count; t++) {
                    addUnique(&target.states, &target.count, tl->destinations[t]);
                }
            }

            if (target.count > 0) {
                int foundIdx = -1;
                for (int existing = 0; existing < num_subsets; existing++) {
                    if (areSubsetsEqual(&target, &subsets[existing])) {
                        foundIdx = existing; break;
                    }
                }
                if (foundIdx == -1) {
                    foundIdx = num_subsets;
                    subsets[num_subsets++] = target;
                } else {
                    free(target.states);
                }
                tempTrans[processed][sym] = foundIdx;
            } else {
                 free(target.states);
            }
        }
        processed++;
    }

    if (!createAutomaton(out, num_subsets, A->num_symbols)) {
        // cleanup
        for(int i=0; i<num_subsets; i++) free(subsets[i].states);
        for(int i=0; i<capacity; i++) free(tempTrans[i]);
        free(subsets);
        free(tempTrans);
        return false;
    }
    out->num_initials = 1;
    out->initials = malloc(sizeof(int));
    if (!out->initials) {
        freeAutomaton(out);
        // cleanup
        for(int i=0; i<num_subsets; i++) free(subsets[i].states);
        for(int i=0; i<capacity; i++) free(tempTrans[i]);
        free(subsets);
        free(tempTrans);
        return false;
    }
    out->initials[0] = 0;

    for(int i=0; i<num_subsets; i++) {
        bool isFinal = false;
        for(int k=0; k<subsets[i].count; k++) {
            if (arrayContains(A->finals, A->num_finals, subsets[i].states[k])) {
                isFinal = true; break;
            }
        }
        if (isFinal) {
            int *new_finals = realloc(out->finals, (out->num_finals + 1) * sizeof(int));
            if (!new_finals) {
                freeAutomaton(out);
                // cleanup
                for(int j=0; j<num_subsets; j++) free(subsets[j].states);
                for(int j=0; j<capacity; j++) free(tempTrans[j]);
                free(subsets);
                free(tempTrans);
                return false;
            }
            out->finals = new_finals;
            out->finals[out->num_finals++] = i;
        }
        for(int sym=0; sym<A->num_symbols; sym++) {
            if (tempTrans[i][sym] != -1) {
                if (!addTransition(out, i, sym, tempTrans[i][sym])) {
                    freeAutomaton(out);
                    // cleanup
                    for(int j=0; j<num_subsets; j++) free(subsets[j].states);
                    for(int j=0; j<capacity; j++) free(tempTrans[j]);
                    free(subsets);
                    free(tempTrans);
                    return false;
                }
            }
        }
        free(subsets[i].states);
        free(tempTrans[i]);
    }
    free(subsets);
    free(tempTrans);
    return true;
}

bool minimize(const Automaton *A, Automaton *out, FILE *logFile) {
    // Assume A is DFA
    int n = A->num_states;
    bool **distinguishable = malloc(n * sizeof(bool*));
    if (!distinguishable) return false;
    for (int i = 0; i < n; i++) {
        distinguishable[i] = calloc(n, sizeof(bool)); // false by default
        if (!distinguishable[i]) {
            for (int j = 0; j < i; j++) free(distinguishable[j]);
            free(distinguishable);
            return false;
        }
    }

    // Mark distinguishable if one final, one not
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            bool i_final = arrayContains(A->finals, A->num_finals, i);
            bool j_final = arrayContains(A->finals, A->num_finals, j);
            if (i_final != j_final) {
                distinguishable[i][j] = true;
                distinguishable[j][i] = true;
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (!distinguishable[i][j]) {
                    for (int sym = 0; sym < A->num_symbols; sym++) {
                        int idx_i = i * A->num_symbols + sym;
                        int idx_j = j * A->num_symbols + sym;
                        int dest_i = A->transitions[idx_i].count > 0 ? A->transitions[idx_i].destinations[0] : -1;
                        int dest_j = A->transitions[idx_j].count > 0 ? A->transitions[idx_j].destinations[0] : -1;
                        if (dest_i != dest_j && (dest_i == -1 || dest_j == -1 || distinguishable[dest_i][dest_j])) {
                            distinguishable[i][j] = true;
                            distinguishable[j][i] = true;
                            changed = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Now, group indistinguishable states
    int *group = malloc(n * sizeof(int));
    if (!group) {
        for (int i = 0; i < n; i++) free(distinguishable[i]);
        free(distinguishable);
        return false;
    }
    int num_groups = 0;
    bool *visited = calloc(n, sizeof(bool));
    if (!visited) {
        free(group);
        for (int i = 0; i < n; i++) free(distinguishable[i]);
        free(distinguishable);
        return false;
    }

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            group[i] = num_groups;
            for (int j = i + 1; j < n; j++) {
                if (!distinguishable[i][j]) {
                    group[j] = num_groups;
                    visited[j] = true;
                }
            }
            num_groups++;
            visited[i] = true;
        }
    }

    // Create minimized automaton
    if (!createAutomaton(out, num_groups, A->num_symbols)) {
        free(visited);
        free(group);
        for (int i = 0; i < n; i++) free(distinguishable[i]);
        free(distinguishable);
        return false;
    }

    // Set initials
    out->num_initials = A->num_initials;
    out->initials = malloc(out->num_initials * sizeof(int));
    if (!out->initials) {
        freeAutomaton(out);
        free(visited);
        free(group);
        for (int i = 0; i < n; i++) free(distinguishable[i]);
        free(distinguishable);
        return false;
    }
    for (int i = 0; i < A->num_initials; i++) {
        out->initials[i] = group[A->initials[i]];
    }

    // Set finals
    bool *group_final = calloc(num_groups, sizeof(bool));
    if (!group_final) {
        freeAutomaton(out);
        free(visited);
        free(group);
        for (int i = 0; i < n; i++) free(distinguishable[i]);
        free(distinguishable);
        return false;
    }
    for (int i = 0; i < A->num_finals; i++) {
        group_final[group[A->finals[i]]] = true;
    }
    for (int g = 0; g < num_groups; g++) {
        if (group_final[g]) {
            int *new_finals = realloc(out->finals, (out->num_finals + 1) * sizeof(int));
            if (!new_finals) {
                free(group_final);
                freeAutomaton(out);
                free(visited);
                free(group);
                for (int i = 0; i < n; i++) free(distinguishable[i]);
                free(distinguishable);
                return false;
            }
            out->finals = new_finals;
            out->finals[out->num_finals++] = g;
        }
    }

    // Set transitions
    for (int g = 0; g < num_groups; g++) {
        for (int sym = 0; sym < A->num_symbols; sym++) {
            // Find a representative state in the group
            int rep = -1;
            for (int s = 0; s < n; s++) {
                if (group[s] == g) {
                    rep = s;
                    break;
                }
            }
            if (rep != -1) {
                int idx = rep * A->num_symbols + sym;
                if (A->transitions[idx].count > 0) {
                    int dest = A->transitions[idx].destinations[0];
                    int dest_group = group[dest];
                    if (!addTransition(out, g, sym, dest_group)) {
                        free(group_final);
                        freeAutomaton(out);
                        free(visited);
                        free(group);
                        for (int i = 0; i < n; i++) free(distinguishable[i]);
                        free(distinguishable);
                        return false;
                    }
                }
            }
        }
    }

    free(group_final);
    free(visited);
    free(group);
    for (int i = 0; i < n; i++) free(distinguishable[i]);
    free(distinguishable);
    return true;
}