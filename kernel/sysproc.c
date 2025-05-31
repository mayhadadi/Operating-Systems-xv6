#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "peterson.h"

uint64
sys_exit(void)
{
  int n;
  char msg[32];
  
  
  argint(0, &n);
  // Initialize to empty string
  msg[0] = '\0';
  
  // Try to get the message - IMPORTANT: use address 1 for the second argument
  argstr(1, msg, sizeof(msg));
  
  // Debug print
  //printf("sys_exit received message: '%s'\n", msg);
  
  exit(n, msg);
  return 0;  // Not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  uint64 msg_addr;
  argaddr(0, &p);
  argaddr(1, &msg_addr);
  
  return wait(p, msg_addr);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}
uint64
sys_memsize(void)
{
  struct proc *p = myproc();
  
  // Return the size of the process's memory
  return p->sz;
}
uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
uint64
sys_forkn(void)
{
  int n;
  uint64 pids_addr;
  
  // Get number of child processes to create
  argint(0, &n);
  
  
  // Get pointer to store PIDs
  argaddr(1, &pids_addr);
  
  
  return forkn(n, pids_addr);
}
uint64
sys_waitall(void)
{
  uint64 n_addr;
  uint64 statuses_addr;
  
  // Get pointer to store number of finished processes
  
    argaddr(0, &n_addr);
  // Get pointer to store exit statuses
  
   argaddr(1, &statuses_addr);
  return waitall(n_addr, statuses_addr);
}

uint64
sys_peterson_create(void)
{
    // Find an unused lock slot
    for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
        if (__sync_lock_test_and_set(&peterson_locks[i].active, 1) == 0) {
            // Found unused slot, initialize it
            peterson_locks[i].flag[0] = 0;
            peterson_locks[i].flag[1] = 0;
            peterson_locks[i].turn = 0;
            peterson_locks[i].lock_id = next_lock_id++;
            
            __sync_synchronize(); // Ensure initialization is visible
            return peterson_locks[i].lock_id;
        }
    }
    return -1; // No available slots
}

uint64
sys_peterson_acquire(void)
{
    int lock_id, role;
    
    // Get arguments
    argint(0, &lock_id);
    argint(1, &role);
    
    // Validate role
    if (role != 0 && role != 1)
        return -1;
    
    // Find the lock
    struct peterson_lock *lock = 0;
    for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
        if (peterson_locks[i].active && peterson_locks[i].lock_id == lock_id) {
            lock = &peterson_locks[i];
            break;
        }
    }
    
    if (!lock)
        return -1;
    
    // Peterson's algorithm with yielding
    while (1) {
        __sync_lock_test_and_set(&lock->flag[role], 1);
        __sync_lock_test_and_set(&lock->turn, 1 - role);
        
        __sync_synchronize(); // Memory barrier
        
        // Check if we can enter critical section
        if (!lock->flag[1 - role] || lock->turn == role) {
            break; // Got the lock
        }
        
        // Reset our flag and yield
        __sync_lock_release(&lock->flag[role]);
        yield(); // Give up CPU to other processes
    }
    
    return 0;
}

uint64
sys_peterson_release(void)
{
    int lock_id, role;
    
    // Get arguments
    argint(0, &lock_id);
    argint(1, &role);
    
    // Validate role
    if (role != 0 && role != 1)
        return -1;
    
    // Find the lock
    struct peterson_lock *lock = 0;
    for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
        if (peterson_locks[i].active && peterson_locks[i].lock_id == lock_id) {
            lock = &peterson_locks[i];
            break;
        }
    }
    
    if (!lock)
        return -1;
    
    // Release the lock
    __sync_lock_release(&lock->flag[role]);
    __sync_synchronize();
    
    return 0;
}

uint64
sys_peterson_destroy(void)
{
    int lock_id;
    
    // Get argument
    argint(0, &lock_id);
    
    // Find and destroy the lock
    for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
        if (peterson_locks[i].active && peterson_locks[i].lock_id == lock_id) {
            peterson_locks[i].flag[0] = 0;
            peterson_locks[i].flag[1] = 0;
            peterson_locks[i].turn = 0;
            peterson_locks[i].lock_id = 0;
            
            __sync_synchronize();
            __sync_lock_release(&peterson_locks[i].active);
            return 0;
        }
    }
    
    return -1; // Lock not found
}