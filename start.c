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
void process_pas(int pasCount, char* isVip);
void process_dispatcher(int plane_count, int planes_size);
void process_plane(int plane_count, int planes_size);
void process_check();
void process_check2();
void soft_finish();
void koniec(int signal);

int msgID, msqid_ci, shmID, semID, ilosc_pasazerow, ilosc_VIP;
pid_t dispatcher_pid, *pas_pid;

int main(){
  //inicjalizacja zmiennych
  int ilosc_samolotow, ilosc_miejsc, i;
  int *shm;
  char isVip;

  //obsługa Ctrl+C
  struct sigaction action;
  action.sa_handler = koniec;
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  //soft finish
  sigset_t block_mask;
  sigemptyset(&block_mask);
  sigaddset(&block_mask, SIGINT);
  sigaddset(&block_mask, SIGTERM);

  struct sigaction sig0;
  sig0.sa_handler = soft_finish;
  sig0.sa_mask = block_mask;
  sig0.sa_flags = 0;
  sigaction(0, &sig0, NULL);

  //pobieranie parametrów startowych
  //init_airport(&ilosc_pasazerow, &ilosc_VIP, &ilosc_samolotow, &ilosc_miejsc);
  ilosc_pasazerow = 100;
  ilosc_VIP = 10;
  ilosc_samolotow = 5;
  ilosc_miejsc = 10;

  pas_pid = (pid_t *)malloc((ilosc_pasazerow + ilosc_VIP) * sizeof(pid_t));

  //inicjalizacja IPC
  msgID = kolejka_init(get_key('K'), IPC_CREAT | IPC_EXCL | 0600);
  msqid_ci = kolejka_init(get_key('C'), IPC_CREAT | IPC_EXCL | 0600);
  shmID = pamiec_init(get_key('M'), (ilosc_samolotow*ilosc_miejsc+1)*sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
  semID = sem_init(get_key('S'), SEM_NUM, IPC_CREAT | IPC_EXCL | 0600);

  //ustawianie IPC
  for(i=0; i<SEM_NUM; i++) sem_setval(semID, i, 0);
  shm = pamiec_add(shmID);
  if(ilosc_VIP != 0) shm[ilosc_samolotow*ilosc_miejsc] = ilosc_pasazerow + ilosc_VIP;
  else shm[ilosc_samolotow*ilosc_miejsc] = ilosc_pasazerow;

  //start procesów
  isVip = '0';
  process_pas(ilosc_pasazerow, &isVip);
  isVip = '1';
  process_pas(ilosc_VIP, &isVip);
  //process_plane(ilosc_samolotow, ilosc_miejsc);
  process_dispatcher(ilosc_samolotow, ilosc_miejsc);
  //process_check();
  //process_check2();

  //czeka na koniec dyspozytora
  waitpid(dispatcher_pid, NULL, 0);
  //for(i=0; i<ilosc_pasazerow+ilosc_VIP; i++) wait(NULL);
  kill(getpid(), 0);
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

void process_pas(int pasCount, char* isVip){
  pid_t pid;

  for(int i=0; i < pasCount; i++){
    pid = fork();
    switch(pid){
      case -1:
        perror("start.c | Pasazerowie | fork | ");
        exit(1);

      case 0:
        if (execl("./pasazer", "pasazer", isVip, NULL) == -1) {
          perror("start.c | Pasazerowie | execl | ");
          exit(2);
        }

      default:
        pas_pid[i] = pid;
    }
  }
}

void process_dispatcher(int plane_count, int planes_size){
  char plane_num_c[11];
  char planes_size_c[11];
  sprintf(plane_num_c, "%d", plane_count);
  sprintf(planes_size_c, "%d", planes_size);

  dispatcher_pid = fork();
  switch(dispatcher_pid){
    case -1:
      perror("start.c | Dyspozytor | fork | ");
      exit(1);

    case 0:
      if (execl("./dyspozytor", "dyspozytor", plane_num_c, planes_size_c, NULL) == -1) {
          perror("start.c | Dyspozytor | execl | ");
          exit(2);
        }

    default:
      printf("Start: Dyspozytor posiada PID -> %d\n", dispatcher_pid);
      fflush(stdout);
  }
}

void koniec(int sig){
  printf("Start: Koniec z sygnalem %d\n", sig);

  if(sig != 0) {
    kill(dispatcher_pid, SIGUSR2);
    waitpid(dispatcher_pid, NULL, 0);
  }

  for(int i=0; i<ilosc_pasazerow+ilosc_VIP; i++) kill(pas_pid[i], SIGINT);
  kolejka_remove(msgID);
  kolejka_remove(msqid_ci);
  pamiec_remove(shmID);
  sem_remove(semID, SEM_NUM);
  exit(sig);
}

void soft_finish() {
  printf("Start: KONIEC");

  for(int i=0; i<ilosc_pasazerow+ilosc_VIP; i++) kill(pas_pid[i], SIGINT);
  kolejka_remove(msgID);
  kolejka_remove(msqid_ci);
  pamiec_remove(shmID);
  sem_remove(semID, SEM_NUM);
  exit(0);
}