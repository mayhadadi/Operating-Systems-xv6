#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int lock_id = peterson_create();
    if (lock_id < 0) {
        printf("Failed to create lock\n");
        exit(1, "");  // Your exit() requires 2 arguments from Assignment 1
    }
    
    printf("Created Peterson lock with ID: %d\n", lock_id);
    
    int fork_ret = fork();
    if (fork_ret < 0) {
        printf("Fork failed\n");
        exit(1, "");
    }
    
    int role = fork_ret > 0 ? 0 : 1;
    char *process_name = role == 0 ? "Parent" : "Child";
    
    for (int i = 0; i < 10; i++) {
        if (peterson_acquire(lock_id, role) < 0) {
            printf("Failed to acquire lock\n");
            exit(1, "");
        }
        
        // Critical section
        printf("%s process (role %d) in critical section - iteration %d\n", 
               process_name, role, i);
        
        // Simulate some work
        for (volatile int j = 0; j < 1000000; j++);
        
        if (peterson_release(lock_id, role) < 0) {
            printf("Failed to release lock\n");
            exit(1, "");
        }
        
        // Some work outside critical section
        for (volatile int j = 0; j < 500000; j++);
    }
    
    if (fork_ret > 0) {
        // Parent waits for child and destroys lock
        char exit_msg[32];
        wait(0, exit_msg);  // Your wait() requires 2 arguments from Assignment 1
        printf("Parent process destroying lock\n");
        if (peterson_destroy(lock_id) < 0) {
            printf("Failed to destroy lock\n");
            exit(1, "");
        }
    }
    
    exit(0, "");  // Success exit
}