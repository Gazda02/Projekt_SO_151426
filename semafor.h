#ifndef SEMAFORS_H
#define SEMAFORS_H

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include <errno.h>

int semafor_init(key_t key, int num_of_sem, int flag);
void semafor_setval(int semid, int semnum, int semval);
int semafor_getval(int semid, int semnum);
void semafor_wait(int semid, int semnum);
int semafor_remove(int semid, int semnum);

#endif //SEMAFORS_H
