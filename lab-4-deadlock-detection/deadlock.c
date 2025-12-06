#include <stdio.h>
#include <stdlib.h>

#define NUM_PROCS 15
#define NUM_RES   15

// process p is waiting on resource r 
int waits[NUM_PROCS][NUM_RES];

// r is held by p 
// r is free
int held_by[NUM_RES];

void init_matrix(void) {
    for (int p = 0; p < NUM_PROCS; p++) {
        for (int r = 0; r < NUM_RES; r++) {
            waits[p][r] = 0;
        }
    }
    for (int r = 0; r < NUM_RES; r++) {
        held_by[r] = -1;
    }
}

void acquire(int p, int r) {
    // r free -> p acquires it
    if (held_by[r] == -1) {
        held_by[r] = p;
    } else {
        // Resource is held so the p must wait
        waits[p][r] = 1;
    }
}

int release_res(int r) {
    // frees the resource
    held_by[r] = -1;

    // find the first process (p) waiting 
    int next_proc = -1;
    for (int p = 0; p < NUM_PROCS; p++) {
        if (waits[p][r] == 1) {
            next_proc = p;
            break;
        }
    }

    if (next_proc != -1) {
        // the next process now acquires r
        waits[next_proc][r] = 0;
        held_by[r] = next_proc;
    }

    return next_proc; // returns a -1 if no one waiting
}

int search_process(int visited[], int start, int current_p);
int search_resource(int visited[], int start, int current_r);

int start_search(int start_p) {
    int visited[NUM_PROCS];
    for (int i = 0; i < NUM_PROCS; i++) {
        visited[i] = 0;
    }
    return search_process(visited, start_p, start_p);
}

int search_process(int visited[], int start, int current_p) {
    if (visited[current_p]) {
        return 0; // if it has already bene visited
    }

    visited[current_p] = 1;

    // follows all waiting edges
    for (int r = 0; r < NUM_RES; r++) {
        if (waits[current_p][r]) {
            if (search_resource(visited, start, r)) {
                return 1; 
            }
        }
    }

    return 0; 
}

int search_resource(int visited[], int start, int current_r) {
    int holder = held_by[current_r];

    if (holder == -1) {
        // if there is no edge r -> p
        return 0;
    }

    // if resource leads back to the start process, a loop is found
    if (holder == start) {
        return 1;
    }

    // otherwise it keeps exploring
    if (!visited[holder]) {
        if (search_process(visited, start, holder)) {
            return 1;
        }
    }

    return 0;
}

int main(void) {
    int p, r;
    char op;

    init_matrix();

    while (scanf("%d %c %d", &p, &op, &r) == 3) {
        if (p < 0 || p >= NUM_PROCS || r < 0 || r >= NUM_RES) {
            continue;
        }

        if (op == 'a') {
            // resource aquires r by p
            acquire(p, r);

            // checks if there is a deadlock, starting at p
            if (start_search(p)) {
                printf("Deadlock detected involving process %d\n", p);
                return -1;
            }

        } else if (op == 'r') {
            // releases resource r
            int next = release_res(r);

            if (next != -1) {
                if (start_search(next)) {
                    printf("Deadlock detected involving process %d\n", next);
                    return -1;
                }
            }

        } else {
            continue;
        }
    }

    // returns if there is no deadlock detected
    return 0;
}
