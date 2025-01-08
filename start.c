#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "semafor.h"
#include "komunikat.h"
#include "pamiec.h"
#include "params.h"

void init_airport(int* pas, int* pasVip, int* plane, int* planesSize);
void thread_pas(int pasCount, char* isVip);
void thread_plane(int planeCount, int *planesSize);
void koniec(int signal);

int msgID, shmID, semID;

int main(){
  int ilosc_pasazerow, ilosc_samolotow, ilosc_VIP, ilosc_miejsc;
  char isVip;

  //obsługa Ctrl+C
  struct sigaction action;
  action.sa_handler = koniec;
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);

  init_airport(&ilosc_pasazerow, &ilosc_VIP, &ilosc_samolotow, &ilosc_miejsc);

  msgID = kolejka_init(get_key(".", 'K'), IPC_CREAT | IPC_EXCL | 0600);
  shmID = pamiec_init(get_key(".", 'M'), ilosc_samolotow*ilosc_miejsc*sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
  semID = semafor_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | IPC_EXCL | 0600);
  for(int i=0; i<SEM_NUM; i++) semafor_setval(semID, i, 0);

  isVip = '0';
  thread_pas(ilosc_pasazerow, &isVip);
  isVip = '1';
  thread_pas(ilosc_VIP, &isVip);
  thread_plane(ilosc_samolotow, ilosc_miejsc);
}

void init_airport(int *pas, int *pasVip, int *plane, int* planesSize){
  printf("===================\nInicjalizacaja lotniska\n===================\n");
  printf("Podaj liczbe pasazerow (bez VIP): ");
  scanf("%i", pas);

  printf("Podaj liczbe VIP: ");
  scanf("%i", pasVip);

  printf("Podaj liczbe samolotow: ");
  scanf("%i", plane);

  printf("Podaj pojemnosc samolotu: ");
  scanf("%i", planesSize);
}

void thread_pas(int pasCount, char* isVip){
  for(int i=0; i < pasCount; i++){
    switch(fork()){
      case -1:
        perror("start.c | Pasazerowie | fork | ");
        exit(1);

      case 0:
        if (execl("./pasazer", "pasazer", isVip, NULL) == -1) {
          perror("start.c | Pasazerowie | execl | ");
          exit(2);
        }

      default:
        NULL;
    }
  }
}

void thread_plane(int planeCount, int planesSize){
  char planeSize[3];
  char planeNum[2];
  char shmSize[5];
  sprintf(planeSize, "%d", planesSize);
  sprintf(planeSize, "%d", planesSize*planeCount);

  for(int i=0; i < planeCount; i++){
    switch(fork()){
      case -1:
        perror("start.c | Samoloty | fork | ");
        exit(1);

      case 0:
        sprintf(planeNum, "%d", i);
        if(execl("./kapitan", "kapitan", planeNum, planeSize, shmSize, NULL) == -1){
          perror("start.c | Samoloty | execl | ");
          exit(2);
        }

      default:
        NULL;
    }
  }
}

void koniec(int sig) {
  kolejka_remove(msgID);
  pamiec_remove(shmID);
  semafor_remove(semID, SEM_NUM);
  exit(9);
}