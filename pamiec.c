#include <stdio.h>
#include <stdlib.h>
#include "pamiec.h"

int pamiec_init(key_t key, size_t size, int flag){
  int shmid = shmget(key, size, flag);
  if (shmid == -1){
    perror("pamiec.c | pamiec_init | ");
    exit(1);
  }
  return shmid;
}

int* pamiec_add(int shmid){
  int *ptr = (int*)shmat(shmid, NULL, 0);
  if (ptr == (void*)-1){
    perror("pamiec.c | pamiec_add | ");
    exit(1);
  }
  return ptr;
}

int pamiec_remove(int shmid){
  int result = shmctl(shmid, IPC_RMID, NULL);
  if (result == -1) perror("pamiec.c | pamiec_remove | ");
  return result;
}