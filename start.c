#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(){
  int ilosc_pasazerow;
  int ilosc_samolotow;
  int ilosc_VIP;

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
        perror("Error | Start | Pasazerowie | fork() |");
        exit(1);

      case 0:
        if(execl("./pasazer", "pasazer", NULL) == -1){
          perror("Error | Start | Pasazerowie | execl() |");
          exit(2);
        }
    }
  }

  for(int i=0; i < ilosc_samolotow; i++){
    switch(fork()){
      case -1:
        perror("Error | Start | Samoloty | fork() |");
        exit(1);

      case 0:
        if(execl("./kapitan", "kapitan", NULL) == -1){
          perror("Error | Start | Samoloty | execl() |");
          exit(2);
        }
    }
  }

  for(int i=0; i < ilosc_VIP; i++){
    switch(fork()){
      case -1:
        perror("Error | Start | VIP | fork() |");
        exit(1);

      case 0:
        if(execl("./pasazer", "pasazer", NULL) == -1){
          perror("Error | Start | VIP | execl() |");
          exit(2);
        }
    }
  }
}

int stworz(int ilosc, char* rodzaj){


}
