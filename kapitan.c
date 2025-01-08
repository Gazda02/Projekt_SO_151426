#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pamiec.h"
#include "komunikat.h"
#include "semafor.h"
#include "params.h"

int main(int argc, char *argv[]){
  key_t key_msg, key_shm;
  struct radio radio_msg;
  int shmIndex, msgID, shmID, semID, maxMasaBagazu, iloscMiejsc, pojSchody;

  if(argc != 4){
    perror("kapitan.c | innit | ");
    exit(1);
  }
  shmIndex = atoi(argv[1]) * atoi(argv[2]);
  iloscMiejsc = atoi(argv[2]);
  pojSchody = (iloscMiejsc % 10) + 5;

  srand(time(NULL));
  maxMasaBagazu = (rand() % (MAX_MASA_BAGAZU-MIN_MASA_BAGAZU)) + MIN_MASA_BAGAZU;

  shmID = pamiec_init(get_key(".", 'M'), atoi(argv[3])*sizeof(int), IPC_CREAT | 0200);
  semID = semafor_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);

  msgID = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0400);
  radio_msg.mtype = RADIO_TYPE;
  radio_msg.dane = maxMasaBagazu;

  while(1){
    kolejka_send(msgID, &radio_msg, sizeof(radio_msg.dane));
    semafor_wait(semID, MARSHALLING);

    semafor_setval(semID, TO_PLANE, iloscMiejsc);
    semafor_setval(semID, TO_STAIRS, pojSchody);

    while(semafor_getval(semID, TO_PLANE) != 0){
      if (semafor_getval(semID, TO_STAIRS) == 0) semafor_setval(semID, TO_STAIRS, pojSchody);
    }
  }
}