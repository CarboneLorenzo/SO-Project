#include "bit_map_buddy.h"
#include <stdio.h>

#define BUFFER_SIZE 102400
#define BUDDY_LEVELS 9
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS))

char buffer[BUFFER_SIZE]; // 100 Kb buffer to handle memory should be enough
char memory[MEMORY_SIZE];

BuddyAllocator alloc;
int main(int argc, char** argv) {

  //1 we see if we have enough memory for the buffers
  int req_size=BuddyAllocator_calcSize(BUDDY_LEVELS);
  printf("size requested for initialization: %d/BUFFER_SIZE\n", req_size);

  //2 we initialize the allocator
  printf("init... ");
  BuddyAllocator_init(&alloc, BUDDY_LEVELS,
                      buffer,
                      BUFFER_SIZE,
                      memory,
                      MIN_BUCKET_SIZE);
  printf("DONE\n");
  
  //successful tests:
  void* p1=BuddyAllocator_malloc(&alloc, 100);
  void* p5=BuddyAllocator_malloc(&alloc, 100);
  BuddyAllocator_free(&alloc, p1);
  BuddyAllocator_free(&alloc, p5);

  void* p4=BuddyAllocator_malloc(&alloc, 5000);
  void* p2=BuddyAllocator_malloc(&alloc, 10000);
  void* p7=BuddyAllocator_malloc(&alloc, 5000);
  void* p3=BuddyAllocator_malloc(&alloc, 100000);
  BuddyAllocator_free(&alloc, p2);
  BuddyAllocator_free(&alloc, p3);
  BuddyAllocator_free(&alloc, p7);
  BuddyAllocator_free(&alloc, p4);

  //failing tests:
  void* p8=BuddyAllocator_malloc(&alloc, 640000);
  void* p9=BuddyAllocator_malloc(&alloc, 640000);
  BuddyAllocator_free(&alloc, p8);
  BuddyAllocator_free(&alloc, p9);

  void* p6=BuddyAllocator_malloc(&alloc, 1000000000);
  BuddyAllocator_free(&alloc, p6);

  BuddyAllocator_free(&alloc, p1);
  
  return 0;
  
}


