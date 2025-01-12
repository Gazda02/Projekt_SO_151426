#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "komunikat.h"
#include "semafor.h"
#include "params.h"

int main(int argc, char *argv[]) {
  //inicjalizacja zmiennych
  int semid, msqid, msqid_ci, masa_bagazu, is_vip, kontrabanda, decision;
  int my_pid = getpid();
  AirHostess airHostess_msg;
  CheckIn checkIn_msg;
  Radio radio_msg, radio_null;

  //ustawianie zmiennych
  if(argc != 2){
    perror("pasazer.c | innit | ");
    exit(1);
  }
  is_vip = atoi(argv[1]);

  srand(time(NULL));
  masa_bagazu = rand() % MAX_MASA_BAGAZU;

  checkIn_msg.type = CHECK_IN;
  checkIn_msg.pas_pid = my_pid;
  checkIn_msg.lug_wt = masa_bagazu;

  airHostess_msg.type = GET_SEAT;
  airHostess_msg.pid = my_pid;

  radio_msg.radioType = RADIO_WAIT;

  //inicjalizacja IPC
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);
  msqid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);
  msqid_ci = kolejka_init(get_key(".", 'C'), IPC_CREAT | 0600);

  while(1){
    if(is_vip == 0) {
      kolejka_send(msqid_ci, &checkIn_msg, sizeof(checkIn_msg.lug_wt) + sizeof(checkIn_msg.pas_pid));
      kolejka_recv(msqid_ci, &radio_msg, sizeof(checkIn_msg.lug_wt), my_pid);
      if(checkIn_msg.lug_wt == 0) continue;
      printf("IDEEE\n");

      //printf("sem TO_PLANE\n");
      //fflush(stdout);
      if(sem_nowait(semid, TO_PLANE) == -1){
        kolejka_send(msqid, &radio_msg, sizeof(radio_msg.radioType));
        printf("wracam\n");
        kolejka_recv(msqid, &radio_null, sizeof(radio_null.radioType), RADIO_UNLUCKY);
        continue;
      }
      printf("IDEEE2\n");

      //TODO kontrola bezpieczeństwa
    }
    else sem_wait(semid, TO_PLANE);

    //printf("sem TO_STAIRS\n");
    //fflush(stdout);
    if(sem_nowait(semid, TO_STAIRS) == -1){
      decision = 1;
      kolejka_send(msqid, &radio_msg, sizeof(radio_msg.radioType));

      while(sem_nowait(semid, TO_STAIRS) == -1){
        if(kolejka_recv_noblock(msqid, &radio_null, sizeof(radio_null.radioType), RADIO_UNLUCKY) == 0) decision = 0;
      }

      if(!decision) continue;
      kolejka_recv(msqid, &radio_null, sizeof(radio_null.radioType), RADIO_WAIT);
      printf("Ide dalej!!!\n");
      fflush(stdout);
    }

    kolejka_send(msqid, &airHostess_msg, sizeof(airHostess_msg.pid));

    //printf("sem TO_SEAT\n");
    //fflush(stdout);
    //sem_wait(semid, TO_SEAT);
    kolejka_recv(msqid, &airHostess_msg, sizeof(airHostess_msg.pid), my_pid);
    printf("Siedze\n");
    fflush(stdout);

    while(1){}
  }
}
