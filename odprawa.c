#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "semafor.h"
#include "komunikat.h"
#include "params.h"

void signal2();

bool is_airport_open;

int main(){
  //zmienne
  int semid, msqid, msqid_ci, aktualna_masa_bagazu;
  size_t msg_size;
  Radio radio_msg;
  CheckIn checkin_msg;

  //obsluga sygnalow
  struct sigaction sig2;
  sig2.sa_handler = signal2;
  sig2.sa_flags = SA_RESTART;

  sigaction(SIGUSR2, &sig2, NULL);

  //inicjalizacja IPC
  msqid = kolejka_init(get_key('K'), IPC_CREAT | 0600);
  msqid_ci = kolejka_init(get_key('C'), IPC_CREAT | 0600);
  semid = sem_init(get_key('S'), SEM_NUM, IPC_CREAT | 0600);

  //ustawianie zmiennych
  is_airport_open = true;
  msg_size = sizeof(checkin_msg.lug_wt) + sizeof(checkin_msg.pas_pid);

  //pobranie masy bagazu
  kolejka_recv(msqid, &radio_msg, sizeof(radio_msg.data), RADIO_READY);
  aktualna_masa_bagazu = radio_msg.data;
  printf("Odprawa: Start\n");

  while(is_airport_open){
    //odboir komunikatu od pasazera
    if(kolejka_recv_noblock(msqid_ci, &checkin_msg, msg_size, 0) != -1){

      //weryfikacja masy bagazu
      if(checkin_msg.lug_wt > aktualna_masa_bagazu) radio_msg.data = 0;
      else radio_msg.data = 1;
      radio_msg.radioType = checkin_msg.pas_pid;

      //przekazanie decyzji pasazerowi
      usleep(300);
      kolejka_send(msqid_ci, &radio_msg, sizeof(radio_msg.data));
    }

    //sprawdzenie czy nie podstawia nie nowy samolot
    if(sem_nowait(semid, CHECKS) != -1) {
      if(kolejka_recv(msqid, &radio_msg, sizeof(radio_msg.data), RADIO_READY) == -1) printf("Odprawa: Error\n");
      aktualna_masa_bagazu = radio_msg.data;
      printf("Odprawa: Nowa masa bagazu -> %d\n", aktualna_masa_bagazu);
      fflush(stdout);
    }
  }

  printf("Odprawa: Zamknięcie stanowiska\n");
  exit(0);
}

void signal2(){
  is_airport_open = false;
}