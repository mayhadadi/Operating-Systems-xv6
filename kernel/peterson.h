#ifndef PETERSON_H
#define PETERSON_H

#define MAX_PETERSON_LOCKS 15

struct peterson_lock {
    int active;           // Whether this lock slot is in use
    int flag[2];         // Peterson algorithm flags for process 0 and 1
    int turn;            // Peterson algorithm turn variable
    int lock_id;         // Unique identifier for this lock
};

// External declarations (not definitions)
extern struct peterson_lock peterson_locks[MAX_PETERSON_LOCKS];
extern int next_lock_id;

// Function declaration
void peterson_init(void);

#endif