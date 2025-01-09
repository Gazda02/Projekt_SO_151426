#ifndef SEMAFORS_H
#define SEMAFORS_H

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include <errno.h>

int sem_init(key_t key, int num_of_sem, int flag);
void sem_setval(int semid, int semnum, int semval);
int sem_getval(int semid, int semnum);
int sem_getwait(int semid, int semnum);
void sem_wait(int semid, int semnum);
int sem_remove(int semid, int semnum);

#endif //SEMAFORS_H
