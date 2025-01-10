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
  int my_pid = getpid();
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
  airHostess_msg.pid = my_pid;

  //inicjalizacja IPC
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);
  msgid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);

  //printf("Pasazer %d: start\n", getpid());

  printf("sem TO_PLANE\n");
  printf("Pasazer: TO_PLANE ustawione na %d\n", sem_getval(semid, TO_PLANE));
  fflush(stdout);
  sem_wait(semid, TO_PLANE);

  printf("sem TO_STAIRS\n");
  fflush(stdout);
  sem_wait(semid, TO_STAIRS);
  kolejka_send(msgid, &airHostess_msg, sizeof(airHostess_msg.pid));

  printf("sem TO_SEAT\n");
  fflush(stdout);
  //sem_wait(semid, TO_SEAT);
  kolejka_recv(msgid, &airHostess_msg, sizeof(airHostess_msg.pid), my_pid);
  printf("Siedze\n");
  fflush(stdout);

  while(1) sleep(1);

}
