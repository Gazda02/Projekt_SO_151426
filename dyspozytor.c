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
  msgid = kolejka_init(get_key('K'), IPC_CREAT | 0400);
  semid = sem_init(get_key('S'), SEM_NUM, IPC_CREAT | 0600);

  //ustawnienie semafora dla kontroli
  sem_setval(semid, CHECKS, 0);

  printf("Dyspozytor: Start\n");
  while(1){
    //wyslanie inforamcji dla pilotow o wolnym stanowisku
    printf("Dyspozytor: Wolne stanowsiko\n");
    radio_msg.radioType = RADIO_TAXIING;
    kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));

    //TODO logika dyspozytora

    //odebranie inforamcji o odlocie
    kolejka_recv(msgid, &radio_null, sizeof(radio_msg.data), RADIO_TAKEOFF);

    //czasowe zamknięcie odprawy
    sem_setval(semid, CHECKS, 3);
    sem_setval(semid, COME_BACK, 100);

    //odeslanie pasazerow ktorzy nie zdarzyli wsiasc
    sleep(3);

    //ponowne otwarcie odprawy
    sem_setval(semid, CHECKS, 0);
    sem_setval(semid, COME_BACK, 0);
  }
}