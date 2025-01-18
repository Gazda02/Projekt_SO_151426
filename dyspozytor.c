#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "komunikat.h"
#include "semafor.h"
#include "params.h"

//tmp
#include <unistd.h>
#include <sys/wait.h>

#include "pamiec.h"

void process_plane();
void process_check();
void process_check2();
void signal1();
void signal2();

pid_t check_pid, check2_pid, current_pid, *planes_pid;
int planes_size, planes_count, shm_size;

int main(int argc, char *argv[]){
  int msgid, semid, shmid, pas_i, *shm;
  Radio radio_msg;
  Radio2 radio2_msg;
  Radio radio_null;

  //obsluga sygnalow
  struct sigaction sig1;
  sig1.sa_handler = signal1;
  sig1.sa_flags = SA_RESTART;

  struct sigaction sig2;
  sig2.sa_handler = signal2;
  sig2.sa_flags = SA_RESTART;

  sigaction(SIGUSR1, &sig1, NULL);
  sigaction(SIGUSR2, &sig2, NULL);

  if(argc != 3) {
    perror("dyspozytor.c | init |\n");
    exit(5);
  }
  planes_count = atoi(argv[1]);
  planes_size = atoi(argv[2]);
  pas_i = planes_size * planes_count;
  shm_size = pas_i + 1;

  planes_pid = (pid_t*)malloc(planes_count * sizeof(pid_t));

  //inicjalizacja IPC
  msgid = kolejka_init(get_key('K'), IPC_CREAT | 0600);
  semid = sem_init(get_key('S'), SEM_NUM, IPC_CREAT | 0600);
  shmid = pamiec_init(get_key('M'), shm_size * sizeof(int), IPC_CREAT | 0400);

  shm = pamiec_add(shmid);

  //start procesow lotniska
  process_plane();
  process_check();
  process_check2();

  //ustawnienie semafora dla kontroli
  sem_setval(semid, CHECKS, 0);

  printf("Dyspozytor: Start\n");
  while(1){
    //wyslanie inforamcji dla pilotow o wolnym stanowisku
    printf("Dyspozytor: Wolne stanowsiko\n");
    radio_msg.radioType = RADIO_TAXIING;
    kolejka_send(msgid, &radio_msg, sizeof(radio_msg.data));
    //czeka na potwierdzenie od pilota
    kolejka_recv(msgid, &radio2_msg, sizeof(radio2_msg.pid), RADIO_COPY);
    current_pid = radio2_msg.pid;

    //odebranie inforamcji o odlocie
    while(kolejka_recv_noblock(msgid, &radio_null, sizeof(radio_msg.data), RADIO_TAKEOFF) == -1) {}
    if(shm[pas_i] == 0) break;


    //czasowe zamknięcie odprawy
    sem_setval(semid, CHECKS, 3);
    sem_setval(semid, COME_BACK, 100);

    //odeslanie pasazerow ktorzy nie zdarzyli wsiasc
    sleep(3);

    //ponowne otwarcie odprawy
    sem_setval(semid, CHECKS, 0);
    sem_setval(semid, COME_BACK, 0);
  }

  signal2();
}

void process_plane(){
  pid_t plane_pid;
  char planes_size_c[11];
  char planes_num_c[11];
  char shm_size_c[12];
  sprintf(planes_size_c, "%d", planes_size);
  sprintf(shm_size_c, "%d", shm_size);

  for(int i=0; i < planes_count; i++){
    plane_pid = fork();
    switch(plane_pid){
      case -1:
        perror("dyspozytor.c | Samoloty | fork | ");
      exit(1);

      case 0:
        sprintf(planes_num_c, "%d", i);
        if(execl("./kapitan", "kapitan", planes_num_c, planes_size_c, shm_size_c, NULL) == -1){
          perror("dyspozytor.c | Samoloty | execl | ");
          exit(2);
        }

      default:
        planes_pid[i] = plane_pid;
    }
  }
}

void process_check(){
  check_pid = fork();
  switch(check_pid){
    case -1:
      perror("dyspozytor.c | Odprawa | fork | ");
      exit(1);

    case 0:
      if (execl("./odprawa", "odprawa", NULL) == -1){
        perror("dyspozytor.c | Odprawa | execl | ");
        exit(2);
      }

    default:
      NULL;
  }
}

void process_check2(){
  check2_pid = fork();
  switch(check2_pid){
    case -1:
      perror("dyspozytor.c | Kontrola osobista | fork | ");
      exit(1);

    case 0:
      if (execl("./kontrola_osobista", "kontrola_osobista", NULL) == -1){
        perror("dyspozytor.c | Kontrola osobista | execl | ");
        exit(2);
      }

    default:
      NULL;
  }
}

void signal1() {
  kill(current_pid, SIGUSR1);
}

void signal2() {
  int i;

  printf("Dyspozytor: Zamykam lotnisko\n");

  kill(check_pid, SIGUSR2);
  kill(check2_pid, SIGUSR2);
  for(i=0; i<planes_count; i++) kill(planes_pid[i], SIGUSR2);

  for(i=0; i<planes_count+2; i++) wait(NULL);

  exit(0);
}