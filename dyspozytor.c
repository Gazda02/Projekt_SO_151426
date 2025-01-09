#include <stdio.h>
#include <stdlib.h>
#include "komunikat.h"

//tmp
#include <unistd.h>

int main(){
  int msg_id;
  Radio radio_msg;
  Radio radio_null;

  msg_id = kolejka_init(get_key(".", 'K'), IPC_CREAT | 0400);

  printf("Dyspozytor: start\n");
  while(1){
    printf("Dyspozytor: wolne stanowsiko\n");
    radio_msg.radioType = RADIO_TAXIING;
    kolejka_send(msg_id, &radio_msg, sizeof(radio_msg.data));

    //TODO

    kolejka_recv(msg_id, &radio_null, sizeof(radio_msg.data), RADIO_TAKEOFF);
  }
}