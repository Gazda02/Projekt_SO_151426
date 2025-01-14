#include <stdio.h>
#include <unistd.h>
#include "semafor.h"
#include "komunikat.h"
#include "params.h"

int main(){
  //zmienne
  int semid, msqid, msqid_ci, aktualna_masa_bagazu;
  size_t msg_size;
  Radio radio_msg;
  CheckIn checkin_msg;

  //inicjalizacja IPC
  msqid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);
  msqid_ci = kolejka_init(get_key(".", 'C'), IPC_CREAT | 0600);
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);

  //ustawianie zmiennych
  msg_size = sizeof(checkin_msg.lug_wt) + sizeof(checkin_msg.pas_pid);

  //pobranie masy bagazu
  kolejka_recv(msqid, &radio_msg, sizeof(radio_msg.data), RADIO_READY);
  aktualna_masa_bagazu = radio_msg.data;
  printf("Odprawa: Start\n");

  while(1){
    //odboir komunikatu od pasazera
    if(kolejka_recv_noblock(msqid_ci, &checkin_msg, msg_size, 0) != -1){

      //weryfikacja masy bagazu
      if(checkin_msg.lug_wt > aktualna_masa_bagazu) radio_msg.data = 0;
      else radio_msg.data = 1;
      radio_msg.radioType = checkin_msg.pas_pid;

      //przekazanie decyzji pasazerowi
      sleep(1);
      kolejka_send(msqid_ci, &radio_msg, sizeof(radio_msg.data));
    }

    //sprawdzenie czy nie podstawia nie nowy samolot
    if(sem_nowait(semid, CHECKS) != 0) {
      kolejka_recv(msqid, &radio_msg, sizeof(radio_msg.data), RADIO_READY);
      aktualna_masa_bagazu = radio_msg.data;
      printf("Odprawa: Nowa masa bagazu -> %d\n", aktualna_masa_bagazu);
      fflush(stdout);
    }
  }
}