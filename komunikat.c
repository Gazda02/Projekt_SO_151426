#include <stdio.h>
#include <stdlib.h>
#include "komunikat.h"

key_t get_key(char id){
  key_t key = ftok(".", id);
  if(key == -1){
    perror("komunikat.c | ftok | ");
    exit(1);
  }
  return key;
}

int kolejka_init(key_t key, int flag){
  int msgid = msgget(key, flag);

  if (msgid == -1){
    perror("kominikat.c | kolejka_init | ");
    exit(2);
  }
  return msgid;
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

int kolejka_recv_noblock(int msqid, void *msg, size_t len, long int type){
  return msgrcv(msqid, msg, len, type, IPC_NOWAIT);
}

int kolejka_count(int msqid, long int type){
  struct msqid_ds buf;

  if (msgctl(msqid, IPC_STAT, &buf) == -1) {
    perror("kominikat.c | kolejka_count | ");
  }

  // Teraz można przeglądać kolejkę, aby policzyć tylko komunikaty określonego typu
  int count = 0;
  struct msgbuf {
    long mtype;       // Typ komunikatu
    int mtext;    // Treść (mały bufor tylko dla sprawdzania typów)
  } message;

  while (msgrcv(msqid, &message, sizeof(message.mtext), type, IPC_NOWAIT) != -1) {
    count++;
  }

  // Przywrócenie komunikatów z powrotem do kolejki
  for (int i = 0; i < count; i++) {
    msgsnd(msqid, &message, sizeof(message.mtext), IPC_NOWAIT);
  }

  return count;
}

int kolejka_remove(int msqid){
  int result = msgctl(msqid, IPC_RMID, NULL);
  if (result == -1) perror("kominikat.c | kolejka_remove | ");
  return result;
}
