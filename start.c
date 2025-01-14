#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "semafor.h"
#include "komunikat.h"
#include "pamiec.h"
#include "params.h"

void init_airport(int* pas, int* pasVip, int* plane, int* planesSize);
void thread_pas(int pasCount, char* isVip);
void thread_plane(int planeCount, int planesSize);
void thread_dispatcher();
void thread_check();
void koniec(int signal);

int msgID, msqid_ci, shmID, semID;

int main(){
  //inicjalizacja zmiennych
  int ilosc_pasazerow, ilosc_samolotow, ilosc_VIP, ilosc_miejsc, i;
  int *shm;
  char isVip;

  //obsługa Ctrl+C
  struct sigaction action;
  action.sa_handler = koniec;
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  //pobieranie parametrów startowych
  //init_airport(&ilosc_pasazerow, &ilosc_VIP, &ilosc_samolotow, &ilosc_miejsc);
  ilosc_pasazerow = 12;
  ilosc_VIP = 0;
  ilosc_samolotow = 2;
  ilosc_miejsc = 5;

  //inicjalizacja IPC
  msgID = kolejka_init(get_key(".", 'K'), IPC_CREAT | IPC_EXCL | 0600);
  msqid_ci = kolejka_init(get_key(".", 'C'), IPC_CREAT | IPC_EXCL | 0600);
  shmID = pamiec_init(get_key(".", 'M'), (ilosc_samolotow*ilosc_miejsc+1)*sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
  semID = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | IPC_EXCL | 0600);

  //ustawianie IPC
  for(i=0; i<SEM_NUM; i++) sem_setval(semID, i, 0);
  shm = pamiec_add(shmID);
  if(ilosc_VIP != 0) shm[ilosc_samolotow*ilosc_miejsc] = ilosc_pasazerow + ilosc_VIP;
  else shm[ilosc_samolotow*ilosc_miejsc] = ilosc_pasazerow;

  //start procesów
  isVip = '0';
  thread_pas(ilosc_pasazerow, &isVip);
  isVip = '1';
  thread_pas(ilosc_VIP, &isVip);
  thread_plane(ilosc_samolotow, ilosc_miejsc);
  thread_dispatcher();
  thread_check();

  for(i=0; i<ilosc_pasazerow+ilosc_VIP; i++) wait(NULL);
  koniec(0);
}

void init_airport(int *pas, int *pasVip, int *plane, int *planesSize){
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
  char planeSize[11];
  char planeNum[11];
  char shmSize[12];
  sprintf(planeSize, "%d", planesSize);
  sprintf(shmSize, "%d", planesSize*planeCount+1);

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

void thread_dispatcher(){
  switch(fork()){
    case -1:
      perror("start.c | Dyspozytor | fork | ");
      exit(1);

    case 0:
      if (execl("./dyspozytor", "dyspozytor", NULL) == -1) {
          perror("start.c | Dyspozytor | execl | ");
          exit(2);
        }

    default:
        NULL;
  }
}

void thread_check(){
  switch(fork()){
    case -1:
      perror("start.c | Kontrola | fork | ");
      exit(1);

    case 0:
      if (execl("./odprawa", "odprawa", NULL) == -1){
        perror("start.c | Kontrola | execl | ");
        exit(2);
      }

  	default:
		NULL;
  }
}

void koniec(int sig){
  kolejka_remove(msgID);
  kolejka_remove(msqid_ci);
  pamiec_remove(shmID);
  sem_remove(semID, SEM_NUM);
  exit(sig);
}