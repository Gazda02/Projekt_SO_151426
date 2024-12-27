#ifndef KOMUNIKAT_H
#define KOMUNIKAT_H

#include <sys/msg.h>

int kolejka_init(key_t key, int flag);
int kolejka_send(int msqid, void *msg, size_t len);
int kolejka_recv(int msqid, void *msg, size_t len, long int type);
int kolejka_remove(int msqid);

#endif //KOMUNIKAT_H
