#include <stdio.h>
#include <stdlib.h>
#include "semafor.h"

int semafor_init(key_t key, int num_of_sem, int flag){
  int semid = semget(key, num_of_sem, flag);

  if (semid == -1){
    perror("semafor.c | semafor_init | ");
    exit(1);
  }
  return semid;
}

void semafor_setval(int semid, int semnum, int semval){
  if (semctl(semid, semnum, SETVAL, semval, NULL) == -1){
    perror("semafor.c | semafor_setval | ");
    exit(1);
  }
}

int semafor_getval(int semid, int semnum){
  int semval = semctl(semid, semnum, GETVAL, NULL);

  if (semval == -1){
    perror("semafor.c | semafor_getval | ");
    exit(1);
  }
  return semval;
}

void semafor_wait(int semid, int semnum){
  struct sembuf operacje[1];
  operacje[0].sem_num = semnum;
  operacje[0].sem_op = -1;
  operacje[0].sem_flg = SEM_UNDO;

  if (semop(semid, operacje, 1) == -1)
  {
    perror("semafor.c | semafor_wait | ");
    exit(1);
  }
}

int semafor_remove(int semid, int num_of_sem){
  int result = semctl(semid, num_of_sem, IPC_RMID, NULL);
  if (result == -1) perror("semafor.c | semafor_remove | ");
  return result;
}
