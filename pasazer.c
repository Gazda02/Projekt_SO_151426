#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "komunikat.h"
#include "semafor.h"
#include "params.h"

int main(int argc, char *argv[]) {
  //inicjalizacja zmiennych
  int semid, msgid, masa_bagazu, is_vip, kontrabanda;
  AirHostess airHostess_msg;

  //ustawianie zmiennych
  if(argc != 2){
    perror("pasazer.c | innit | ");
    exit(1);
  }
  is_vip = atoi(argv[1]);

  srand(time(NULL));
  masa_bagazu = rand() % MAX_MASA_BAGAZU;

  airHostess_msg.type = GET_SEAT;
  airHostess_msg.pid = getpid();

  //inicjalizacja IPC
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);
  msgid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);

  //printf("Pasazer %d: start\n", getpid());

  //printf("sem TO_PLANE\n");
  //fflush(stdout);
  sem_wait(semid, TO_PLANE);
  sem_wait(semid, TO_STAIRS);
  kolejka_send(msgid, &airHostess_msg, sizeof(airHostess_msg.pid));
  sem_wait(semid, TO_SEAT);
  printf("Siedze\n");
  fflush(stdout);

  while(1) sleep(1);

}
