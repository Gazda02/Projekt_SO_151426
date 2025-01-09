#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "pamiec.h"
#include "komunikat.h"
#include "semafor.h"
#include "params.h"

void assign_seats(int semid, int msgid, int *free_seat, int *shm_seats);

int main(int argc, char *argv[]){
  //inicjalizacja zmiennych
  Radio radio_msg;
  Radio radio_null;
  int pilot_no, shm_index, msgid, shmid, semid, max_masa_bagazu, ilosc_miejsc, poj_schody, seat;
  int* passengers;

  //ustawianie zmiennych p1
  if(argc != 4){
    perror("kapitan.c | innit | ");
    exit(1);
  }
  pilot_no = atoi(argv[1]);
  ilosc_miejsc = atoi(argv[2]);
  shm_index = pilot_no * ilosc_miejsc;
  poj_schody = 2; //ilosc_miejsc % 10 + 5;

  //ustawianie zmiennych p2
  srand(time(NULL));
  max_masa_bagazu = rand() % (MAX_MASA_BAGAZU-MIN_MASA_BAGAZU) + MIN_MASA_BAGAZU;

  //inicjalizacja IPC
  shmid = pamiec_init(get_key(".", 'M'), atoi(argv[3])*sizeof(int), IPC_CREAT | 0600);
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);
  msgid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);
  passengers = pamiec_add(shmid);

  printf("Kapitan %d: start\n", pilot_no);

  //program zasadniczy
  while(1){
    radio_msg.radioType = RADIO_READY;
    radio_msg.data = max_masa_bagazu;
    kolejka_recv(msgid, &radio_null, sizeof(radio_msg.data), RADIO_TAXIING);
    kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));

    printf("Kapitan %d: ustawia sie\n", pilot_no);
    fflush(stdout);
    while(sem_getwait(semid, TO_PLANE) == 0) {sleep(1);}
    printf("Kapitan %d: otwieram bramki\n", pilot_no);
    fflush(stdout);
    sem_setval(semid, TO_STAIRS, poj_schody);
    sem_setval(semid, TO_PLANE, ilosc_miejsc);

    seat = shm_index;
    sleep(1);

    //do przemyślenia
    while(sem_getval(semid, TO_PLANE) != 0 || sem_getwait(semid, TO_STAIRS) > 0) {
      // || sem_getwait(semid, TO_PLANE) != 0){

      if (sem_getval(semid, TO_STAIRS) == 0){
        assign_seats(semid, msgid, &seat, passengers);
        sleep(1);
        sem_setval(semid, TO_STAIRS, poj_schody);
      }
    }
    assign_seats(semid, msgid, &seat, passengers);

    printf("Kapitan %d: odlatuje\n", pilot_no);
    fflush(stdout);
    radio_msg.radioType = RADIO_TAKEOFF;
    radio_msg.data = 0;
    kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));

    for(int i=shm_index; i<seat; i++) {kill(passengers[i], SIGTERM); printf("Zabito %d\n", passengers[i]);}
    sleep(10);

  }
}


void assign_seats(int semid, int msgid, int *free_seat_ptr, int *shm_seats) {
  AirHostess airHostess_msg;
  int waiting_pas = sem_getwait(semid, TO_SEAT);
  int free_seat_val = *free_seat_ptr;

  for(int i=0; i<waiting_pas; i++){
    kolejka_recv(msgid, &airHostess_msg, sizeof(airHostess_msg.pid), GET_SEAT);
    shm_seats[free_seat_val] = airHostess_msg.pid;
    free_seat_val++;
  }

  *free_seat_ptr = free_seat_val;
  sem_setval(semid, TO_SEAT, waiting_pas);
}