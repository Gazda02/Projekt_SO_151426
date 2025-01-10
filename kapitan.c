#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "pamiec.h"
#include "komunikat.h"
#include "semafor.h"
#include "params.h"

void assign_seats();
void radio_takeoff(bool is_finish);

//zmienne używane przez funkcje
Radio radio_msg;
int pilot_no, msgid, shm_index, seat, semid;
int* passengers;

int main(int argc, char *argv[]){
  //inicjalizacja zmiennych
  Radio radio_null;
  int shmid, max_masa_bagazu, ilosc_miejsc, poj_schody, pas_no;
  struct timespec start_time, current_time;

  //ustawianie zmiennych p1
  if(argc != 4){
    perror("kapitan.c | innit | ");
    exit(1);
  }
  pilot_no = atoi(argv[1]);
  ilosc_miejsc = atoi(argv[2]);
  pas_no = atoi(argv[3]) - 1;
  shm_index = pilot_no * ilosc_miejsc;
  poj_schody = 2; //ilosc_miejsc % 10 + 5;

  //ustawianie zmiennych p2
  srand(time(NULL));
  max_masa_bagazu = rand() % (MAX_MASA_BAGAZU-MIN_MASA_BAGAZU) + MIN_MASA_BAGAZU;

  //inicjalizacja IPC
  shmid = pamiec_init(get_key(".", 'M'), (pas_no+1)*sizeof(int), IPC_CREAT | 0600);
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

    printf("pas left ==> %d\n", passengers[pas_no]);
    if(passengers[pas_no] == 0){
      radio_takeoff(true);
      exit(0);
    }
    printf("Kapitan %d: ustawia sie\n", pilot_no);
    fflush(stdout);

    printf("Kapitan %d: otwieram bramki\n", pilot_no);
    printf("Kapitan %d: TO_PLANE ustawione na %d ilosc mnisjsc -> %d\n", pilot_no, sem_getval(semid, TO_PLANE), ilosc_miejsc);

    fflush(stdout);
    sem_setval(semid, TO_STAIRS, poj_schody);
    sleep(1);
    sem_setval(semid, TO_PLANE, ilosc_miejsc);

    printf("Kapitan %d: TO_PLANE ustawione na %d ilosc mnisjsc -> %d\n", pilot_no, sem_getval(semid, TO_PLANE), ilosc_miejsc);


    seat = shm_index;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while(current_time.tv_sec - start_time.tv_sec < DEPARTURE_DURATION){
      if (sem_getval(semid, TO_STAIRS) == 0){
        sleep(1);
        assign_seats();
        sem_setval(semid, TO_STAIRS, poj_schody);
      }
      clock_gettime(CLOCK_MONOTONIC, &current_time);
    }
    sleep(1);
    assign_seats();

    if(seat-shm_index > ilosc_miejsc) {
      printf("Kapitan %d: samolot przeladowany\n", pilot_no);
      exit(1);
    }
    printf("Kapitan %d: TO_PLANE ustawione na %d\n", pilot_no, sem_getval(semid, TO_PLANE));
    if(sem_getwait(semid, TO_SEAT) != 0) printf("Kapitan %d: ktos jest na chodach\n", pilot_no);
    passengers[pas_no] -= seat-shm_index;
    if(sem_getval(semid, TO_PLANE) > 0) sem_setval(semid, TO_PLANE, 0);
    if(sem_getval(semid, TO_STAIRS) > 0) sem_setval(semid, TO_STAIRS, 0);
    radio_takeoff(false);
    printf("Kapitan %d: TO_PLANE ustawione na %d\n", pilot_no, sem_getval(semid, TO_PLANE));

    for(int i=shm_index; i<seat; i++) {kill(passengers[i], SIGTERM); passengers[i] = 0;}
    sleep(10);
  }
}


void assign_seats() {
  AirHostess airHostess_msg;
  int waiting_pas = kolejka_count(msgid, GET_SEAT);
  printf("Kapitan %d: waiting pas => %d\n", pilot_no, waiting_pas);
  fflush(stdout);
  int pid;

  for(int i=0; i<waiting_pas; i++){
    kolejka_recv(msgid, &airHostess_msg, sizeof(airHostess_msg.pid), GET_SEAT);
    pid = airHostess_msg.pid;
    passengers[seat] = pid;
    airHostess_msg.type = pid;
    kolejka_send(msgid, &airHostess_msg, sizeof(airHostess_msg.pid));
    seat++;
  }

  //sem_setval(semid, TO_SEAT, waiting_pas);
}

void radio_takeoff(bool is_finish) {
  if(is_finish) printf("Kapitan %d: zakonczyl prace\n", pilot_no);
  else printf("Kapitan %d: odlatuje z %d pasazerami\n", pilot_no, seat-shm_index);

  fflush(stdout);
  radio_msg.radioType = RADIO_TAKEOFF;
  kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));
}