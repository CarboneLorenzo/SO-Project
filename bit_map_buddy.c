#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "bit_map_buddy.h"

// these are trivial helpers to support you in case you want
// to do a bitmap implementation
int levelIdx(size_t idx){
  return (int)floor(log2(idx));
};

int buddyIdx(int idx){
  if (idx&0x1){
    return idx-1;
  }
  return idx+1;
}

int parentIdx(int idx){
  return idx/2;
}

int startIdx(int idx){
  return (idx-(1<<levelIdx(idx)));
}


// computes the size in bytes for the allocator
int BuddyAllocator_calcSize(int num_levels) {
  int bitmap_items=(1<<(num_levels+1))-1; // maximum number of allocations, used to determine the max list items
  int bitmap_size=BitMap_getBytes(bitmap_items)+((sizeof(BuddyItem)+sizeof(int))*(bitmap_items));
  return bitmap_size;
}

void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         char* buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size){

  // we need room also for level 0
  alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->min_bucket_size=min_bucket_size;
  assert (num_levels<MAX_LEVELS);
  // we need enough memory to handle internal structures
  assert (buffer_size>=BuddyAllocator_calcSize(num_levels));

  int bitmap_items=1<<(num_levels+1); // maximum number of allocations, used to determine the max list items
  int bitmap_size=BitMap_getBytes(bitmap_items)+((sizeof(BuddyItem)+sizeof(int))*(bitmap_items));

  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d", num_levels);
  printf("\tmax entries %d bytes\n", bitmap_size);
  printf("\tbucket size:%d\n", min_bucket_size);
  printf("\tmanaged memory %d bytes\n", (1<<num_levels)*min_bucket_size);

  // initializing bitmap with default bit = 0
  uint8_t* start_bitmap = buffer;
  BitMap_init(&alloc->bitmap, bitmap_items, start_bitmap);
  
  int i;
  for(i=2; i<bitmap_items; i++) {
    BitMap_setBit(&alloc->bitmap, i, 1);
  }
  BitMap_setBit(&alloc->bitmap, 1, 0);
};

int find_bit(BitMap* bitmap, int level, int actual_level) {
  printf("cerco blocco nel livello: %d\n", actual_level);
  if(actual_level<0) {                        //caso base: nessun buddy abbastanza grande libero->errore
    printf("errore, livello non valido\n");
    return -1;
  }
  int start= 1<<actual_level;                   //definisco inizio e fine degli idx di questo livello
  int stop= 1<<(actual_level+1);                //
  int bit,i;
  printf("start: %d, stop: %d\n", start, stop);
  for(i=start; i<stop; i++) {               //li scandisco per vedere se c'è un bit a 0 (buddy libero)
    bit = BitMap_bit(bitmap, i);
    printf("bit: %d, valore: %d\n",i, bit);
    if(bit==0) {
      printf("trovato bit a livello: %d\n", actual_level);
      if(level==actual_level) return i;         //trovato buddy libero nel livello desiderato
      else{
        BitMap_setBit(bitmap, i, 1);
        BitMap_setBit(bitmap, (i*2)+1, 0);
        BitMap_setBit(bitmap, buddyIdx((i*2)+1), 0);
        return i;                              //ho trovato un buddy libero ma ad un livello più alto, quindi lo divido settandolo a 1 e i figli a 0 e lo ritorno ai livelli ricorsivi precedenti
      }
    }
  }
  printf("non trovato, salgo di livello\n");
  int res = find_bit(bitmap, level, actual_level-1); //non ho trovato un buddy libero a questo livello, quindi vado a quelli sopra
  printf("sceso di livello\n");
  if(res!=-1) {
    int res_child = (res*2)+1;
    if(level==actual_level) {
      printf("trovato");
      return res_child;       //ho diviso vari genitori ed ora ho un bit a 0 nel livello desiderato, lo ritorno
    }else{
      BitMap_setBit(bitmap, res_child, 1);
      BitMap_setBit(bitmap, (res_child*2)+1 , 0);
      BitMap_setBit(bitmap, buddyIdx((res_child*2)+1), 0);
      return res_child;                             //ho un bit a 0 ma sono ancora nei livelli superiori, setto il padre a 1 e i figli a 0 di nuovo e scendo ancora
    }
  }else {                                           //errore nessun buddy libero, ritornato -1 da tutti i livelli ricorsivi
    return -1;
  }   
}

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {
  // we determine the level of the page
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
  int  level=floor(log2(mem_size/(size)));

  // if the level is too small, we pad it to max
  if (level>alloc->num_levels)
    level=alloc->num_levels;

  printf("requested: %d bytes, level %d \n",
         size, level);

  //trova mem
  int idx = find_bit(&alloc->bitmap, level, level);
  if(idx==-1) {
    printf("errore, non c'è abbastanza memoria");
    return NULL;
  }else{
    printf("blocco trovato, idx: %d\n", idx);
    void* start_memory;
    start_memory = alloc->memory + ((idx-(1<<levelIdx(idx))) << (alloc->num_levels-level) )*alloc->min_bucket_size;
    int x;
    int bitmap_items=1<<level;
    for(x=1; x<=bitmap_items; x++) {
      int valore = BitMap_bit(&alloc->bitmap, x);
      printf("bit: %d, valore: %d\n", x, valore);
    }
    return start_memory+8;
  }
}
//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  return; 
}


