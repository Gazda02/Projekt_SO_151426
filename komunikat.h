#ifndef KOMUNIKAT_H
#define KOMUNIKAT_H

#include <sys/msg.h>
//samolot-obsluga
#define RADIO_TAXIING 101
#define RADIO_READY 102
#define RADIO_TAKEOFF 103

#define RADIO_WAIT 407
#define RADIO_UNLUCKY 408
typedef struct{
  long radioType;
  int data;
} Radio;

//samolot-pasazer
#define GET_SEAT 11
typedef struct {
  long type;
  int pid;
} AirHostess;

//odprawa-pasazer
#define CHECK_IN 1
typedef struct {
  long type;
  int pas_pid;
  int lug_wt;
} CheckIn;

key_t get_key(char* path, char id);
int kolejka_init(key_t key, int flag);
int kolejka_send(int msqid, void *msg, size_t len);
int kolejka_recv(int msqid, void *msg, size_t len, long int type);
int kolejka_recv_noblock(int msqid, void *msg, size_t len, long int type);
int kolejka_count(int msqid, long int type);
int kolejka_remove(int msqid);

#endif //KOMUNIKAT_H
