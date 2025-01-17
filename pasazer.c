#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "komunikat.h"
#include "semafor.h"
#include "params.h"

int main(int argc, char *argv[]) {
  //inicjalizacja zmiennych
  int semid, msqid, msqid_ci, masa_bagazu, is_vip, contraband, decision, random;
  unsigned int rand_int;
  int my_pid = (int)getpid();
  AirHostess airHostess_msg;
  CheckIn checkIn_msg;
  Radio radio_msg, radio_null;
  P_Search search_msg;

  //ustawianie zmiennych
  if(argc != 2){
    perror("pasazer.c | innit | ");
    exit(1);
  }
  is_vip = atoi(argv[1]);

  //losowanie masy bagazu
  random = open("/dev/random", O_RDONLY);
  if(random == -1){
    perror("pasazer.c | random | ");
    exit(1);
  }
  if(read(random, &rand_int, sizeof(int)) != sizeof(int)){
    perror("pasazer.c | read random | ");
    exit(1);
  }
  else close(random);
  read(random, &rand_int, sizeof(int));
  masa_bagazu = 0;//rand_int % (MAX_MASA_BAGAZU - MIN_MASA_BAGAZU);

  //losowanie kontrabandy
  if(rand_int % 17 == 0) contraband = 1;
  else contraband = 0;

  //komunikaty - DO POPRAWY
  checkIn_msg.type = CHECK_IN;
  checkIn_msg.pas_pid = my_pid;
  checkIn_msg.lug_wt = masa_bagazu;

  airHostess_msg.type = GET_SEAT;
  airHostess_msg.pid = my_pid;

  radio_msg.radioType = RADIO_WAIT;

  search_msg.type = PRIVATE_SEARCH + (my_pid % 3);
  search_msg.pid = my_pid;
  search_msg.sex = my_pid % 2;
  search_msg.contraband = contraband;

  //inicjalizacja IPC
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);
  msqid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);
  msqid_ci = kolejka_init(get_key(".", 'C'), IPC_CREAT | 0600);

  while(1){
    //wersja dla nie VIP
    if(is_vip == 0) {

      //zakolekowanie siebie do odprawy bagazowej
      kolejka_send(msqid_ci, &checkIn_msg, sizeof(checkIn_msg.lug_wt) + sizeof(checkIn_msg.pas_pid));
      kolejka_recv(msqid_ci, &radio_msg, sizeof(radio_msg.data), my_pid, 'P');
      //powrot na poczatek kolejki
      if(radio_msg.data == 0) continue;

      //kontrola bezpieczeństwa
      //printf("Stanowsiko -> %ld\n", search_msg.type);
      kolejka_send(msqid, &search_msg, 3 * sizeof(int));
      printf("pasazer %d ustawia sie do kontroli\n", my_pid);
      kolejka_recv(msqid, &radio_msg, sizeof(radio_msg.data), my_pid, 'P');
      if(radio_msg.data == 0) {
        search_msg.contraband = 0;
        printf("Pasazer %d: Wyrzucam kontrabande\n", my_pid);
        continue;
      }
      if(radio_msg.data == -1) continue;
      printf("pasazer %d przechodzi kontrole\n", my_pid);

      //jeśli nie zdążyło się na samolot - powrot na poczatek odprawy
      if(sem_nowait(semid, TO_PLANE) == -1){
        printf("pasazer %d czeka na powrot\n", my_pid);
        sem_wait(semid, COME_BACK);
        printf("pasazer %d wraca na poczatek\n", my_pid);
        continue;
      }
      printf("pasazer %d uprawniony do wejscia na samolot\n", my_pid);
    }
    else sem_wait(semid, TO_PLANE); // dla VIP

    //to jest zeby pasazer wrocil na poczatek jezeli samolot odleci
    /*if(sem_nowait(semid, TO_STAIRS) == -1){
      decision = 1;
      //wysyla komunikat ze czeka
      kolejka_send(msqid, &radio_msg, sizeof(radio_msg.data));

      //czeka do poki semafor jest opuszczony
      while(sem_nowait(semid, TO_STAIRS) == -1){
        if(kolejka_recv_noblock(msqid, &radio_null, sizeof(radio_null.data), RADIO_UNLUCKY) == 0){
          decision = 0;
          break;
        }
      }

      //jezeli dostal komunikat UNLUCKY wraca na poczatek
      if(decision == 0) continue;
      //jezeli nie dostal komunikatu sam go odbiera zeby nie smiecic w kolejce
      kolejka_recv(msqid, &radio_null, sizeof(radio_null.data), RADIO_WAIT, 'P');
    }*/

    decision = 1;
    while(sem_nowait(semid, TO_STAIRS) == -1) {
      if(sem_nowait(semid, COME_BACK) != -1) {
        decision = 0;
        break;
      }
    }
    if(decision == 0) continue;

    //wyslanie porsby o miejsce
    kolejka_send(msqid, &airHostess_msg, sizeof(airHostess_msg.pid));
    //potwierdzenie otrzymania miejsce - opcjonalne
    kolejka_recv(msqid, &airHostess_msg, sizeof(airHostess_msg.pid), my_pid, 'P');
    //printf("Pasazer %d: Wsiadlem\n", my_pid);
    fflush(stdout);

    //czeka na koniec podrozy
    while(1){}
  }
}
