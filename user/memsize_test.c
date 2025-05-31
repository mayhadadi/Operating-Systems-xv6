#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int mem_size;
  void *mem_block;

  // (a) Print how many bytes of memory the running process is using
  mem_size = memsize();
  printf("Current memory usage: %d bytes\n", mem_size);

  // (b) Allocate 20k more bytes of memory
  mem_block = malloc(20 * 1024); // 20KB
  if(mem_block == 0) {
    printf("malloc failed\n");
    exit(1, "");
  }
  
  // (c) Print memory usage after allocation
  mem_size = memsize();
  printf("Memory usage after allocation: %d bytes\n", mem_size);
  
  // (d) Free the allocated array
  free(mem_block);
  
  // (e) Print memory usage after release
  mem_size = memsize();
  printf("Memory usage after free: %d bytes\n", mem_size);
  
  exit(0, "");
}