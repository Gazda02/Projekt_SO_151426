#include <stdio.h>
#include <stdlib.h>
#include "komunikat.h"
#include "semafor.h"
#include "params.h"

//tmp
#include <unistd.h>

int main(){
  int msgid, semid;
  Radio radio_msg;
  Radio radio_null;

  //inicjalizacja IPC
  msgid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0400);
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);

  //ustawnienie semafora dla kontroli
  sem_setval(semid, CHECKS, 10000);

  printf("Dyspozytor: start\n");
  while(1){
    //wyslanie inforamcji dla pilotow o wolnym stanowisku
    printf("Dyspozytor: wolne stanowsiko\n");
    radio_msg.radioType = RADIO_TAXIING;
    kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));

    //TODO logika dyspozytora

    //odebranie inforamcji o odlocie
    kolejka_recv(msgid, &radio_null, sizeof(radio_msg.data), RADIO_TAKEOFF);

    //czasowe zamknięcie odprawy
    sem_setval(semid, CHECKS, 0);

    //odeslanie pasazerow ktorzy nie zdarzyli wsiasc
    sleep(2);
    radio_msg.radioType = RADIO_UNLUCKY;
    while(kolejka_recv_noblock(msgid, &radio_null, sizeof(radio_msg.data), RADIO_WAIT) != -1) {
      kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));
    }

    //ponowne otwarcie odprawy
    sem_setval(semid, CHECKS, 10000);
  }
}