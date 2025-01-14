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

  //wylosowanie masy bagazu
  masa_bagazu = my_pid % (MAX_MASA_BAGAZU - MIN_MASA_BAGAZU);

  //komunikaty - DO POPRAWY
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
    //wersja dla nie VIP
    if(is_vip == 0) {

      //zakolekowanie siebie do odprawy bagazowej
      kolejka_send(msqid_ci, &checkIn_msg, sizeof(checkIn_msg.lug_wt) + sizeof(checkIn_msg.pas_pid));
      kolejka_recv(msqid_ci, &radio_msg, sizeof(radio_msg.data), my_pid);
      //powrot na poczatek kolejki
      if(radio_msg.data == 0) continue;

      //TODO kontrola bezpieczeństwa

      //jeśli nie zdążyło się na samolot - powrot na poczatek odprawy
      if(sem_nowait(semid, TO_PLANE) == -1){
        radio_msg.radioType = RADIO_WAIT;
        kolejka_send(msqid, &radio_msg, sizeof(radio_msg.data));
        kolejka_recv(msqid, &radio_null, sizeof(radio_null.data), RADIO_UNLUCKY);
        sleep(3);
        continue;
      }
    }
    else sem_wait(semid, TO_PLANE); // dla VIP

    //to jest zeby pasazer wrocil na poczatek jezeli samolot odleci
    if(sem_nowait(semid, TO_STAIRS) == -1){
      decision = 1;
      //wysyla komunikat ze czeka
      kolejka_send(msqid, &radio_msg, sizeof(radio_msg.radioType));

      //czeka do poki semafor jest opuszczony
      while(sem_nowait(semid, TO_STAIRS) == -1){
        if(kolejka_recv_noblock(msqid, &radio_null, sizeof(radio_null.radioType), RADIO_UNLUCKY) == 0){
          decision = 0;
          break;
        }
      }

      //jezeli dostal komunikat UNLUCKY wraca na poczatek
      if(decision == 0) continue;
      //jezeli nie dostal komunikatu sam go odbiera zeby nie smiecic w kolejce
      kolejka_recv(msqid, &radio_null, sizeof(radio_null.radioType), RADIO_WAIT);
    }

    //wyslanie porsby o miejsce
    kolejka_send(msqid, &airHostess_msg, sizeof(airHostess_msg.pid));
    //potwierdzenie otrzymania miejsce - opcjonalne
    kolejka_recv(msqid, &airHostess_msg, sizeof(airHostess_msg.pid), my_pid);
    printf("Pasazer %d: Wsiadl\n", my_pid);
    fflush(stdout);

    //czeka na koniec podrozy
    while(1){}
  }
}
