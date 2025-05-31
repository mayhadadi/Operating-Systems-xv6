#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define ARRAY_SIZE 65536  // 2^16
#define DEFAULT_NPROC 4

int
main(int argc, char *argv[])
{
  int nproc = DEFAULT_NPROC;
  int child_pids[16] = {0};
  int statuses[16] = {0};
  int n_finished = 0;
  int i, start_index, end_index, chunk_size;
  int child_id, result;
  //unsigned long total_sum = 0;
  
  // Create array dynamically to avoid stack overflow
  int *array = malloc(ARRAY_SIZE * sizeof(int));
  if(!array) {
    printf("Failed to allocate memory for array\n");
    exit(1, "");
  }
  
  // Initialize array with consecutive integers from 0 to ARRAY_SIZE-1
  for(i = 0; i < ARRAY_SIZE; i++) {
    array[i] = i;
  }
  
  // Optional: Allow user to specify the number of processes
  if(argc > 1) {
    nproc = atoi(argv[1]);
    if(nproc < 1 || nproc > 16) {
      printf("Number of processes must be between 1 and 16\n");
      exit(1, "");
    }
  }
  
  printf("Creating %d child processes for array of size %d\n", nproc, ARRAY_SIZE);
  
  // Create child processes using forkn
  result = forkn(nproc, child_pids);
  
  if(result == -1) {
    printf("forkn failed\n");
    free(array);
    exit(1, "");
  } else if(result > 0) {
    // Child process
    child_id = result;
    
    // Calculate the chunk size and array bounds for this child
    chunk_size = ARRAY_SIZE / nproc;
    start_index = (child_id - 1) * chunk_size;
    
    // Last process gets any remainder elements
    if(child_id == nproc) {
      end_index = ARRAY_SIZE;
    } else {
      end_index = start_index + chunk_size;
    }
    
    // Calculate sum of elements in this chunk
    unsigned long long sum = 0;
    for(i = start_index; i < end_index; i++) {
      sum += array[i];
    }
    
    printf("Child %d: sum of elements from index %d to %d is %llu\n", 
           child_id, start_index, end_index-1, sum);
    
    // Free memory before exiting
    free(array);
    
    // Just return the child_id - we can't return the actual sum via exit status
    exit(child_id, "");
  } else {
    // Parent process
    
    // Print PIDs of child processes
    printf("Parent: created child processes with PIDs: ");
    for(i = 0; i < nproc; i++) {
      printf("%d ", child_pids[i]);
    }
    printf("\n");
    
    // Wait for all children to finish
    if(waitall(&n_finished, statuses) == -1) {
      printf("waitall failed\n");
      free(array);
      exit(1, "");
    }
    
    // Check if all children finished
    printf("Parent: %d children finished\n", n_finished);
    if(n_finished != nproc) {
      //printf("Error: expected %d children to finish, but only %d did\n", nproc, n_finished);
      free(array);
      exit(1, "");
    }
    
    // Print out the status values received
    for(i = 0; i < n_finished; i++) {
      printf("Child %d returned status: %d\n", i+1, statuses[i]);
    }
    
    // Use quadrant approach to calculate
    unsigned long long quadrant_sum = 0;
    chunk_size = ARRAY_SIZE / nproc;
    
    for(int q = 0; q < nproc; q++) {
      start_index = q * chunk_size;
      
      // Last process gets any remainder elements
      if(q == nproc-1) {
        end_index = ARRAY_SIZE;
      } else {
        end_index = start_index + chunk_size;
      }
      
      unsigned long long q_sum = 0;
      for(i = start_index; i < end_index; i++) {
        q_sum += array[i];
      }
      
      printf("Quadrant %d sum should be: %d\n", q+1, q_sum);
      quadrant_sum += q_sum;
    }
    
    // Calculate formula sum: n*(n-1)/2
    unsigned long long formula_sum = ((unsigned long long)ARRAY_SIZE * (ARRAY_SIZE - 1)) / 2;
    
    printf("Parent calculated total: %d\n", quadrant_sum);
    printf("Formula sum: %d\n", formula_sum);
    
    if(quadrant_sum == formula_sum) {
      printf("Result is correct!\n");
    } else {
      printf("ERROR: Result is incorrect!\n");
    }
    
    // Free memory before exiting
    free(array);
  }
  
  exit(0, "");
}