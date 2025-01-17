#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
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
  int i, random, shmid, max_masa_bagazu, ilosc_miejsc, poj_schody, pas_no;
  unsigned int rand_int;
  bool in_air;
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

  //losowanie masy bagazu
  random = open("/dev/urandom", O_RDONLY);
  if(random == -1){
    perror("kapitan.c | random | ");
    exit(1);
  }
  if(read(random, &rand_int, sizeof(int)) != sizeof(int)){
    perror("kapitan.c | read random | ");
    exit(1);
  }
  else close(random);
  max_masa_bagazu = 101;//rand_int % (MAX_MASA_BAGAZU-MIN_MASA_BAGAZU) + MIN_MASA_BAGAZU;

  //inicjalizacja IPC
  shmid = pamiec_init(get_key(".", 'M'), (pas_no+1)*sizeof(int), IPC_CREAT | 0600);
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);
  msgid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);
  passengers = pamiec_add(shmid);

  printf("Kapitan %d: start\n", pilot_no);

  //program zasadniczy
  while(1){
    in_air = false;
    radio_msg.radioType = RADIO_READY;
    radio_msg.data = max_masa_bagazu;

    //czeka na pozwolenie na podstawienie sie
    kolejka_recv(msgid, &radio_null, sizeof(radio_msg.data), RADIO_TAXIING, 'K');

    //to sie zobaczy
    if(passengers[pas_no] == 0){
      radio_takeoff(true);
      exit(0);
    }
    printf("Kapitan %d: ustawia sie\n", pilot_no);
    printf("Kapitan %d: otwieram bramki\n", pilot_no);
    fflush(stdout);

    //otwarcie bramek oraz wyslanie inforamcji o masie bagazu
    sem_setval(semid, TO_STAIRS, poj_schody);
    sem_setval(semid, TO_PLANE, ilosc_miejsc);
    kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));
    radio_msg.radioType = RADIO_READY2;
    kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));

    seat = shm_index;

    //czeka na pasazerow przez okreslony czas
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while(current_time.tv_sec - start_time.tv_sec < DEPARTURE_DURATION){

      //jesli schody sie zapelnia to wpuszczamy pasazerow i na nowo otwieramy schody
      if (sem_getval(semid, TO_STAIRS) == 0){
        assign_seats();
        sem_setval(semid, TO_STAIRS, poj_schody);
      }

      clock_gettime(CLOCK_MONOTONIC, &current_time); //do obslugi czasu
    }
    //zamkniecie wyszyskich bramek
    if(sem_getval(semid, TO_PLANE) > 0) sem_setval(semid, TO_PLANE, 0);
    sem_setval(semid, TO_STAIRS, 0);

    //posadzenie ostatnich pasazerow
    assign_seats();

    //sprawdzamy czy kazdy ma miejsce
    if(seat-shm_index > ilosc_miejsc) {
      printf("Kapitan %d: samolot przeladowany\n", pilot_no);
      exit(1);
    }

    //zmniejszenie liczby pozostalych pasazerow
    passengers[pas_no] -= seat-shm_index;

    //wyslanie informacji o odlocie
    radio_takeoff(false);

    //wykonanie lotu
    for(i=shm_index; i<seat; i++) {kill(passengers[i], SIGTERM); passengers[i] = 0;}
    in_air = true;
    sleep(10); //czas lotu
  }
}

//rozdysponowanie miejsc
void assign_seats() {
  AirHostess airHostess_msg;
  int pid;

  while(kolejka_recv_noblock(msgid, &airHostess_msg, sizeof(airHostess_msg.pid), GET_SEAT) != -1){
    pid = airHostess_msg.pid;
    passengers[seat] = pid;
    airHostess_msg.type = pid;
    kolejka_send(msgid, &airHostess_msg, sizeof(airHostess_msg.pid));
    seat++;
  }
}

void radio_takeoff(bool is_finish) {
  if(is_finish) printf("Kapitan %d: zakonczyl prace\n", pilot_no);
  else printf("Kapitan %d: odlatuje z %d pasazerami\n", pilot_no, seat-shm_index);

  fflush(stdout);
  radio_msg.radioType = RADIO_TAKEOFF;
  kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));
}