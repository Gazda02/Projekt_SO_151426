#include <stdio.h>
#include <stdlib.h>
#include "komunikat.h"

int kolejka_init(key_t key, int flag){
  int msqid = msgget(key, flag);

  if (msqid == -1){
    perror("kominikat.c | kolejka_init | ");
    exit(2);
  }
  return msqid;
}

int kolejka_send(int msqid, void *msg, size_t len){
  int result = msgsnd(msqid, msg, len, 0);
  if (result == -1) perror("kominikat.c | kolejka_send | ");
  return result; //to moze jest zbedne
}

int kolejka_recv(int msqid, void *msg, size_t len, long int type){
  int result = msgrcv(msqid, msg, len, type, 0);
  if (result == -1) perror("kominikat.c | kolejka_recv | ");
  return result;
}

int kolejka_remove(int msqid){
  int result = msgctl(msqid, IPC_RMID, NULL);
  if (result == -1) perror("kominikat.c | kolejka_remove | ");
  return result;
}
