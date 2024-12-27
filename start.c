#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "semafor.h"


int main(){
  key_t key_sem, key_shm, key_msg;
  int ilosc_pasazerow;
  int ilosc_samolotow;
  int ilosc_VIP;

  key_sem = ftok(".", 'S');
  key_shm = ftok(".", 'M');
  key_msg = ftok(".", 'K');
  if (key_sem == -1 || key_shm == -1 || key_msg == -1) {
    perror("start.c | ftok | ");
    exit(1);
  }

  printf("===================\nInicjalizacaja lotniska\n===================\n");
  printf("Podaj liczbe pasazerow: ");
  scanf("%i", &ilosc_pasazerow);
  printf("Podaj liczbe samolotow: ");
  scanf("%i", &ilosc_samolotow);
  printf("Podaj liczbe VIP: ");
  scanf("%i", &ilosc_VIP);

  for(int i=0; i < ilosc_pasazerow; i++){
    switch(fork()){
      case -1:
        perror("start.c | Pasazerowie | fork | ");
        exit(1);

      case 0:
        if(execl("./pasazer", "pasazer", NULL) == -1){
          perror("start.c | Pasazerowie | execl | ");
          exit(2);
        }
    }
  }

  for(int i=0; i < ilosc_samolotow; i++){
    switch(fork()){
      case -1:
        perror("start.c | Samoloty | fork | ");
        exit(1);

      case 0:
        if(execl("./kapitan", "kapitan", NULL) == -1){
          perror("start.c | Samoloty | execl | ");
          exit(2);
        }
    }
  }

  for(int i=0; i < ilosc_VIP; i++){
    switch(fork()){
      case -1:
        perror("start | VIP | fork | ");
        exit(1);

      case 0:
        if(execl("./pasazer", "pasazer", NULL) == -1){
          perror("start | VIP | execl | ");
          exit(2);
        }
    }
  }
}

int stworz(int ilosc, char* rodzaj){


}
