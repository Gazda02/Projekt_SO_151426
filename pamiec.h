#ifndef PAMIEC_H
#define PAMIEC_H

#include <sys/shm.h>

int pamiec_init(key_t key, size_t size, int flag);
int* pamiec_add(int shmid);
int pamiec_remove(int shmid);

#endif //PAMIEC_H
