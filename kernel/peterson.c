// kernel/peterson.c
#include "types.h"
#include "peterson.h"

// Global variable definitions (actual storage)
struct peterson_lock peterson_locks[MAX_PETERSON_LOCKS];
int next_lock_id = 1;  // Start from 1, 0 means invalid

// Function definition
void peterson_init(void) {
    for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
        peterson_locks[i].active = 0;
        peterson_locks[i].flag[0] = 0;
        peterson_locks[i].flag[1] = 0;
        peterson_locks[i].turn = 0;
        peterson_locks[i].lock_id = 0;
    }
}