#include <stdio.h>
#include <unistd.h>
#include "semafor.h"
#include "komunikat.h"
#include "params.h"

int main(){
  //zmienne
  int semid, msqid, msqid_ci, aktualna_masa_bagazu;
  Radio radio_msg;
  CheckIn checkin_msg;

  //inicjalizacja IPC
  msqid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);
  msqid_ci = kolejka_init(get_key(".", 'C'), IPC_CREAT | 0600);
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);

  kolejka_recv(msqid, &radio_msg, sizeof(radio_msg.data), RADIO_READY);
  aktualna_masa_bagazu = radio_msg.data;
  printf("Odprawa: Start\n");

  while(1){
    kolejka_recv(msqid_ci, &checkin_msg, sizeof(checkin_msg.lug_wt) + sizeof(checkin_msg.pas_pid), 0);
    if(checkin_msg.lug_wt > aktualna_masa_bagazu) radio_msg.data = 0;
    else radio_msg.data = 1;
    radio_msg.radioType = checkin_msg.pas_pid;
    kolejka_send(msqid_ci, &radio_msg, sizeof(radio_msg.data));

    sleep(1);

    if(sem_new_plane(semid, CHECKS) != 0) {
      printf("Czekam na nowa mase");
      kolejka_recv(msqid, &radio_msg, sizeof(radio_msg.data), RADIO_READY);
      aktualna_masa_bagazu = radio_msg.data;
      printf("Nowa masa bagazu");
    }
  }
}