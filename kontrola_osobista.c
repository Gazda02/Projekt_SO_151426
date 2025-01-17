#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "komunikat.h"
#include "semafor.h"
#include "params.h"

typedef struct {
  long pid;
  int contraband;
  int sex;
  int cut_count;
} Waiting;

void *security_check(void *my_num);
void check(P_Search *pas);
void check_waiting(Waiting *waiting_pas);
void make_waiting(P_Search *pas, Waiting *waiting_pas);
void send_everyone_back(int post_num);
void send_waiting_back(Waiting *waiting_pas);

int msqid, is_new_plane, is_airport_open, size;


int main(){
  int semid, posts[3];
  pthread_t security[3];
  Radio radio_null;

  msqid = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0600);
  semid = sem_init(get_key(".", 'S'), SEM_NUM, IPC_CREAT | 0600);

  size = 3 * sizeof(int);
  is_new_plane = 0;
  is_airport_open = 1;

  kolejka_recv(msqid, &radio_null, sizeof(radio_null.data), RADIO_READY2, 'C');
  sleep(1);

  for(int i = 0; i < 3; i++) {
    posts[i] = i;
    if(pthread_create(&security[i], NULL, security_check, (void *)&posts[i])) {
      perror("kontrola_osobista.c | pthread_create | ");
      exit(1);
    }
  }

  while(is_airport_open) {
    if(sem_nowait(semid, CHECKS) != -1) {
      is_new_plane = 1;
      sleep(3);

      for(int i = 0; i < 3; i++) send_everyone_back(i);

      kolejka_recv(msqid, &radio_null, sizeof(radio_null.data), RADIO_READY2, 'C');
      is_new_plane = 0;
      printf("Kontrola osobista: restart\n");
      fflush(stdout);
    }
  }
}

void *security_check(void *arg) {
  int *my_num, pas_count;
  P_Search pas_l[2], pas, pas_tmp;
  Waiting waiting_pas;

  my_num = (int*)arg;
  waiting_pas.pid = 0;
  pas_count = 0;

  printf("Kontrola osobista %d: Start\n", PRIVATE_SEARCH + *my_num);
  while(is_airport_open){

    //pobranie dwoch pasazerow (o ile sie da)
    while(pas_count < 2 && kolejka_recv_noblock(msqid, &pas_tmp, size, PRIVATE_SEARCH + *my_num) != -1) {
      pas_l[pas_count] = pas_tmp;
      pas_count++;
    }
    //printf("pas_count: %d\n", pas_count);
    for(int i = 0; i < pas_count; i++){printf("pas.pid: %d\n", pas_l[i].pid);}

    //dla pobranych dwoch pasazerow
    if(pas_count == 2) {
      //gdy jest ktos oczekujacy
      if(waiting_pas.pid != 0) {

        //gdy nowopobrani pasazerowie sa tej samej plci
        if (pas_l[0].sex == pas_l[1].sex) {
          for (int i = 0; i < 2; i++) {
            check(&pas_l[i]);
            waiting_pas.cut_count++;
          }
        }
        //gdy nie sa tej samej plci
        else if (pas_l[0].sex == waiting_pas.sex) {
          check_waiting(&waiting_pas);
          check(&pas_l[0]);
          make_waiting(&pas_l[1], &waiting_pas);
          waiting_pas.cut_count = 0;
        }
        else{
          check_waiting(&waiting_pas);
          check(&pas_l[1]);
          make_waiting(&pas_l[0], &waiting_pas);
          waiting_pas.cut_count = 0;
        }

      }
      //gdy nie ma czekajacej osoby
      else {
        printf("2\n");
        fflush(stdout);
        //gdy nowopobrani pasazerowie sa tej samej plci
        if (pas_l[0].sex == pas_l[1].sex) {
          printf("sex1 = %d, sex2 = %d\n", pas_l[0].sex, pas_l[1].sex);
          fflush(stdout);
          for (int i = 0; i < 2; i++) check(&pas_l[i]);
        }
        //gdy nie sa tej samej plci
        else {
          printf("3\n");
          fflush(stdout);
          //gdy uda sie pobrac kolejnego pasazera
          if(kolejka_recv_noblock(msqid, &pas, size , PRIVATE_SEARCH + *my_num) != -1) {
            printf("dobieram\n");
            fflush(stdout);
            //sprawdzenie z kim nowy pasazer dzieli plec
            if(pas_l[0].sex == pas.sex) {
              check(&pas_l[0]);
              check(&pas);
              make_waiting(&pas_l[1], &waiting_pas);
              waiting_pas.cut_count = 1;
            }
            else {
              check(&pas_l[1]);
              check(&pas);
              make_waiting(&pas_l[0], &waiting_pas);
              waiting_pas.cut_count = 1;
            }
          }
          //gdy nie uda sie pobrac kolejnego pasazera
          else {
            check(&pas_l[0]);
            sleep(1);
            check(&pas_l[1]);
          }
        }

      }

      if(waiting_pas.pid != 0 && waiting_pas.cut_count > 1) {
        sleep(1);
        check_waiting(&waiting_pas);
      }

    }
    else if (pas_count == 1) {
      //jest oczekujacy
      if(waiting_pas.pid != 0) {
        if(pas_l[0].sex == waiting_pas.sex) {
          check_waiting(&waiting_pas);
          check(&pas_l[0]);
        }
        else {
          check_waiting(&waiting_pas);
          sleep(1);
          check(&pas_l[0]);
        }
      }
      //nie ma oczekujacego
      else check(&pas_l[0]);
    }
    else {if(waiting_pas.pid != 0) check_waiting(&waiting_pas);}

    sleep(2);
    pas_count = 0;

    while(is_new_plane) {
      if(waiting_pas.pid != 0) send_waiting_back(&waiting_pas);
    }
  }
  return NULL;
}

void check(P_Search *pas) {
  Radio radio_msg;
  radio_msg.radioType = pas->pid;
  radio_msg.data = pas->contraband ? 0 : 1;
  kolejka_send(msqid, &radio_msg, sizeof(radio_msg.data));
  printf("kontrola -> %d -> %d\n", pas->pid, radio_msg.data);
}

void check_waiting(Waiting *waiting_pas) {
  Radio radio_msg;
  radio_msg.radioType = waiting_pas->pid;
  radio_msg.data = waiting_pas->contraband ? 0 : 1;
  kolejka_send(msqid, &radio_msg, sizeof(radio_msg.data));
  printf("kontrola W -> %ld -> %d\n", waiting_pas->pid, radio_msg.data);
  waiting_pas->pid = 0;
}

void make_waiting(P_Search *pas, Waiting *waiting_pas) {
  printf("new w -> %d, old w -> %ld\n", pas->pid, waiting_pas->pid);
  waiting_pas->pid = pas->pid;
  waiting_pas->contraband = pas->contraband;
  waiting_pas->sex = pas->sex;
  waiting_pas->cut_count = 0;
}

void send_everyone_back(int post_num) {
  P_Search pas_tmp;
  Radio radio_msg;
  int i =0;

  while(kolejka_recv_noblock(msqid, &pas_tmp, size, PRIVATE_SEARCH + post_num) != -1) {
    radio_msg.radioType = pas_tmp.pid;
    radio_msg.data = -1;
    kolejka_send(msqid, &radio_msg, sizeof(radio_msg.data));
    i++;
  }
  printf("wrocilo %d pasazerow\n", i);
}

void send_waiting_back(Waiting *waiting_pas) {
  Radio radio_msg;
  radio_msg.radioType = waiting_pas->pid;
  radio_msg.data = -1;
  kolejka_send(msqid, &radio_msg, sizeof(radio_msg.data));
  printf("%ld wraca\n", waiting_pas->pid);
  waiting_pas->pid = 0;
}